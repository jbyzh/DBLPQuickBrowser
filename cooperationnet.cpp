#include "cooperationnet.h"
#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <algorithm>

CooperationNet::CooperationNet(QWidget* parent)
    : QWidget{parent}
{
}

// 清空所有内存数据结构
void CooperationNet::clear()
{
    m_adjList.clear();
}

void CooperationNet::addPublication(const QStringList& authors, const QString& title)
{
    Q_UNUSED(title);
    QStringList normalizedAuthors;
    normalizedAuthors.reserve(authors.size());
    for (const QString& a : authors) {
        const QString trimmed = a.trimmed();
        if (!trimmed.isEmpty()) {
            normalizedAuthors.push_back(trimmed);
        }
    }

    // 创建合作关系图的边，连接作者之间的合作关系,双重循环枚举同一论文里的任意两位作者
    for (int i = 0; i < normalizedAuthors.size(); ++i) {
        for (int j = i + 1; j < normalizedAuthors.size(); ++j) {
            const QString& a = normalizedAuthors[i];
            const QString& b = normalizedAuthors[j];
            if (a == b) {
                continue;
            }
            m_adjList[a][b]++;
            m_adjList[b][a]++;
        }
    }
}

bool CooperationNet::loadFromAuthorIndexDir(const QString& authorIndexDirPath)
{
    clear();
    QDir dir(authorIndexDirPath);
    if (!dir.exists()) {
        return false;
    }

    auto hashAuthorBucket = [](const QString& name) -> int {
        quint64 ans = 0;
        const QByteArray bytes = name.toUtf8();
        for (char c : bytes) {
            ans = ((ans >> 8) & 0xF) ^ ((ans << 4) ^ static_cast<unsigned char>(c));
        }
        return static_cast<int>(ans & 0xFFF); // Data_initial.cpp
    };

    // 读取 author_rank，拿到真实作者名
    QHash<int, QStringList> realAuthorsByBucket;
    const QString rankFilePath = QDir(authorIndexDirPath).absoluteFilePath("../author_rank");
    QFile rankFile(rankFilePath);
    if (rankFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream rin(&rankFile);
        const QRegularExpression re("^\\s*\\d+\\s+(.+)$");
        while (!rin.atEnd()) {
            const QString line = rin.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }
            const QRegularExpressionMatch m = re.match(line);
            if (!m.hasMatch()) {
                continue;
            }
            const QString author = m.captured(1).trimmed();
            if (author.isEmpty()) {
                continue;
            }
            realAuthorsByBucket[hashAuthorBucket(author)].push_back(author);
        }
        rankFile.close();
    }

    // 先按记录ID聚合论文与作者
    QHash<QString, QString> titleByFlag;
    QHash<QString, QSet<QString>> authorsByFlag;
    QHash<int, int> bucketAssignCursor;

    // 遍历所有 author/*.ini 文件
    const QFileInfoList files = dir.entryInfoList(QStringList() << "*.ini", QDir::Files);
    for (const QFileInfo& fi : files) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        bool okBucket = false;
        const int bucketId = fi.baseName().toInt(&okBucket);
        const QStringList bucketAuthors = okBucket ? realAuthorsByBucket.value(bucketId) : QStringList();

        QTextStream in(&f);
        int localLine = 0;
        while (!in.atEnd()) {
            ++localLine;
            const QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            const int pos = line.indexOf('$');
            const QString title = pos > 0 ? line.left(pos).trimmed() : line;
            const QString rawFlag = (pos > 0 ? line.mid(pos + 1).trimmed() : QString());
            const int authorSep = rawFlag.indexOf('#');
            const QString recordIdPart = authorSep >= 0 ? rawFlag.left(authorSep).trimmed() : rawFlag;
            const QString explicitAuthor = authorSep >= 0 ? rawFlag.mid(authorSep + 1).trimmed() : QString();
            const QString recordId = recordIdPart.isEmpty()
                ? QString("NOFLAG_%1_%2").arg(fi.baseName()).arg(localLine)
                : recordIdPart;

            if (!title.isEmpty() && !titleByFlag.contains(recordId)) {
                titleByFlag[recordId] = title;
            }

            QString resolvedAuthor;
            if (!explicitAuthor.isEmpty()) {
                resolvedAuthor = explicitAuthor;
            } else if (!bucketAuthors.isEmpty()) {
                // 冲突桶里按轮转分配，优先确保使用真实作者名
                int& cursor = bucketAssignCursor[bucketId];
                resolvedAuthor = bucketAuthors[cursor % bucketAuthors.size()];
                ++cursor;
            } else {
                // 若缺 author_rank 数据，退化为桶名
                resolvedAuthor = fi.baseName();
            }

            if (!resolvedAuthor.isEmpty()) {
                authorsByFlag[recordId].insert(resolvedAuthor);
            }
        }
        f.close();
    }

    // 按聚合结果建图
    for (auto it = authorsByFlag.constBegin(); it != authorsByFlag.constEnd(); ++it) {
        const QStringList authors = it.value().values();
        if (authors.isEmpty()) {
            continue;
        }
        const QString title = titleByFlag.value(it.key(), it.key());
        addPublication(authors, title);
    }

    return !m_adjList.isEmpty();
}

// 返回某作者与其合作者的邻接表
QHash<QString, int> CooperationNet::getCollaborators(const QString& name)
{
    return m_adjList.value(name.trimmed());
}

// F9功能：给你推荐最可能合作的作者
QVector<QPair<QString, int>> CooperationNet::recommendAuthors(const QString& name, int topN) const
{
    const QString user = name.trimmed();
    if (!m_adjList.contains(user)) {
        return {};
    }
    // 直接合作者（一级好友）
    QSet<QString> direct;
    const auto directMap = m_adjList.value(user);
    for (auto it = directMap.constBegin(); it != directMap.constEnd(); ++it) {
        direct.insert(it.key());
    }
    QHash<QString, int> score;
    // 遍历直接合作者的合作者（二级好友）
    for (const QString& c : direct) {
        const auto secondNeighbors = m_adjList.value(c);
        for (auto it = secondNeighbors.constBegin(); it != secondNeighbors.constEnd(); ++it) {
            const QString cand = it.key();
            // 排除自己 + 已经合作过的
            if (cand == user || direct.contains(cand)) {
                continue;
            }
            // 计算推荐分数：取最小合作次数。和一级好友的合作次数；一级好友和二级好友的合作次数
            score[cand] += qMin(m_adjList.value(user).value(c, 1), it.value());
        }
    }
    // 排序
    QVector<QPair<QString, int>> ranked;
    ranked.reserve(score.size());
    for (auto it = score.constBegin(); it != score.constEnd(); ++it) {
        ranked.push_back({it.key(), it.value()});
    }
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second == b.second) {
            return a.first < b.first;
        }
        return a.second > b.second;
    });
    if (ranked.size() > topN) {
        ranked.resize(topN);
    }
    return ranked;
}

// 计算作者重要性排名（委托给 computePageRank，便于主线程与后台线程共用同一实现）
QVector<QPair<QString, double>> CooperationNet::pageRank(int iterations, double damping) const
{
    return computePageRank(m_adjList, iterations, damping);
}

QVector<QPair<QString, double>> CooperationNet::computePageRank(const QHash<QString, QHash<QString, int>>& adjList,
    int iterations,
    double damping)
{
    QVector<QPair<QString, double>> ret;
    // 与 pageRank 一致，按 keys() 顺序迭代，避免迭代次序变化带来浮点差异
    const QStringList nodeList = adjList.keys();
    QVector<QString> nodes;
    nodes.reserve(nodeList.size());
    for (const QString& k : nodeList) {
        nodes.append(k);
    }
    const int n = nodes.size();
    if (n == 0) {
        return ret;
    }
    QHash<QString, double> pr;
    QHash<QString, double> next;
    // 初始化：所有人分数 = 1/n，所有人平等
    const double init = 1.0 / n;
    for (const QString& node : nodes) {
        pr[node] = init;
    }
    // 传递影响力（复用 next，减少每轮整表分配）
    for (int iter = 0; iter < iterations; ++iter) {
        next.clear();
        for (const QString& node : nodes) {
            next[node] = (1.0 - damping) / n;
        }
        for (const QString& u : nodes) {
            const auto nbrs = adjList.value(u);
            if (nbrs.isEmpty()) {
                continue;
            }
            const double share = damping * pr[u] / nbrs.size();
            for (auto it = nbrs.constBegin(); it != nbrs.constEnd(); ++it) {
                next[it.key()] += share;
            }
        }
        pr.swap(next);
    }

    ret.reserve(pr.size());
    for (auto it = pr.constBegin(); it != pr.constEnd(); ++it) {
        ret.push_back({it.key(), it.value()});
    }
    // 排序
    std::sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    return ret;
}

// 返回所有作者列表
QStringList CooperationNet::allAuthors() const
{
    return m_adjList.keys();
}

bool CooperationNet::hasAuthor(const QString& name) const
{
    return m_adjList.contains(name.trimmed());
}

int CooperationNet::authorCount() const
{
    return m_adjList.size();
}

QHash<QString, QHash<QString, int>> CooperationNet::adjacencyCopy() const
{
    return m_adjList;
}

void CooperationNet::replaceAdjacencyData(const QHash<QString, QHash<QString, int>>& adj)
{
    m_adjList = adj;
}
