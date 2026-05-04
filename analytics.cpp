#include "analytics.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace {

struct AnalyticsCacheEntry
{
    QVector<AuthorStat> topAuthors;
    QMap<int, QVector<KeywordStat>> hotKeywordsByYear;
    QString sourceVersion;
    bool valid = false;
};

QHash<QString, AnalyticsCacheEntry>& analyticsCache()
{
    static QHash<QString, AnalyticsCacheEntry> cache;
    return cache;
}

QString safeLineRead(QTextStream& stream)
{
    QString line = stream.readLine();
    while (!line.isNull() && line.endsWith('\r')) {
        line.chop(1);
    }
    return line;
}

bool authorHeapLess(const AuthorStat& left, const AuthorStat& right)
{
    if (left.paperCount != right.paperCount) {
        return left.paperCount < right.paperCount;
    }
    return left.name > right.name;
}

bool keywordHeapLess(const KeywordStat& left, const KeywordStat& right)
{
    if (left.frequency != right.frequency) {
        return left.frequency < right.frequency;
    }
    return left.word > right.word;
}

bool authorOutputGreater(const AuthorStat& left, const AuthorStat& right)
{
    if (left.paperCount != right.paperCount) {
        return left.paperCount > right.paperCount;
    }
    return left.name < right.name;
}

bool keywordOutputGreater(const KeywordStat& left, const KeywordStat& right)
{
    if (left.frequency != right.frequency) {
        return left.frequency > right.frequency;
    }
    return left.word < right.word;
}

}

TrieTree::TrieTree()
    : m_root(new Node)
{
}

TrieTree::~TrieTree()
{
    deleteNode(m_root);
}

void TrieTree::insert(const QString& word)
{
    Node* current = m_root;
    for (const QChar ch : word) {
        if (!current->children.contains(ch)) {
            current->children.insert(ch, new Node);
        }
        current = current->children.value(ch);
    }
    current->isWord = true;
    ++current->count;
}

void TrieTree::collect(QVector<KeywordStat>& output) const
{
    QString path;
    collectNode(m_root, path, output);
}

void TrieTree::collectNode(Node* node, QString& path, QVector<KeywordStat>& output) const
{
    if (!node) {
        return;
    }
    if (node->isWord) {
        output.push_back({path, node->count});
    }
    for (auto it = node->children.cbegin(); it != node->children.cend(); ++it) {
        path.push_back(it.key());
        collectNode(it.value(), path, output);
        path.chop(1);
    }
}

void TrieTree::deleteNode(Node* node)
{
    if (!node) {
        return;
    }
    for (auto child : node->children) {
        deleteNode(child);
    }
    delete node;
}

AnalyticsService::AnalyticsService(const QString& xmlPath)
{
    setXmlPath(xmlPath);
}

void AnalyticsService::setXmlPath(const QString& xmlPath)
{
    m_xmlPath = xmlPath;
    m_baseDirectory = normalizeBaseDirectory(xmlPath);
    m_loaded = false;
}

QString AnalyticsService::baseDirectory() const
{
    return m_baseDirectory;
}

QString AnalyticsService::lastError() const
{
    return m_lastError;
}

bool AnalyticsService::ensureAnalytics()
{
    if (m_loaded) {
        return true;
    }

    m_lastError.clear();
    if (m_baseDirectory.isEmpty()) {
        m_lastError = QString::fromUtf8("未找到有效的数据目录。");
        return false;
    }

    QFileInfo finishFile(QDir(m_baseDirectory).filePath("database/finish.db"));
    if (!finishFile.exists()) {
        m_lastError = QString::fromUtf8("请先在首页完成 dblp.xml 解析。");
        return false;
    }

    if (tryLoadFromMemoryCache()) {
        loadAuthorCountsFromRankFile();
        m_loaded = true;
        return true;
    }

    if (tryLoadFromResultFiles()) {
        loadAuthorCountsFromRankFile();
        updateMemoryCache();
        m_loaded = true;
        return true;
    }

    loadStopWords();
    loadAuthorStats();
    if (!m_lastError.isEmpty()) {
        return false;
    }

    loadHotspotStats();
    if (!m_lastError.isEmpty()) {
        return false;
    }

    writeAuthorResultFile(m_topAuthors);
    writeHotspotResultFile();
    updateMemoryCache();
    m_loaded = true;
    return true;
}

QVector<AuthorStat> AnalyticsService::topAuthors(int limit) const
{
    return m_topAuthors.mid(0, std::min(limit, static_cast<int>(m_topAuthors.size())));
}

QVector<AuthorStat> AnalyticsService::searchAuthors(const QString& keyword, int limit) const
{
    QVector<AuthorStat> result;
    const QString lowered = keyword.trimmed().toLower();
    if (lowered.isEmpty()) {
        return topAuthors(limit);
    }

    const QVector<QPair<QString, int>> orderedAuthors = m_authorCounts.inOrder();
    for (const auto& entry : orderedAuthors) {
        if (entry.first.toLower().contains(lowered)) {
            result.push_back({entry.first, entry.second});
        }
    }
    std::sort(result.begin(), result.end(), authorOutputGreater);
    if (result.size() > limit) {
        result.resize(limit);
    }
    return result;
}

QList<int> AnalyticsService::availableYears() const
{
    return m_hotKeywordsByYear.keys();
}

QVector<KeywordStat> AnalyticsService::keywordsForYear(int year) const
{
    return m_hotKeywordsByYear.value(year);
}

QMap<QString, QVector<QPair<int, int>>> AnalyticsService::keywordTrends(const QList<int>& years, int topPerYear) const
{
    QMap<QString, QVector<QPair<int, int>>> trends;
    for (const int year : years) {
        const QVector<KeywordStat> keywords = m_hotKeywordsByYear.value(year);
        const int count = std::min(topPerYear, static_cast<int>(keywords.size()));
        for (int index = 0; index < count; ++index) {
            trends[keywords[index].word].push_back(qMakePair(year, keywords[index].frequency));
        }
    }
    return trends;
}

QString AnalyticsService::normalizeBaseDirectory(const QString& rawPath) const
{
    if (rawPath.trimmed().isEmpty()) {
        return QString();
    }

    QFileInfo info(rawPath);
    if (info.isFile()) {
        return QDir::cleanPath(info.absolutePath());
    }
    if (info.isDir()) {
        return QDir::cleanPath(info.absoluteFilePath());
    }

    const QString cleaned = QDir::cleanPath(rawPath);
    QFileInfo guessedFile(QDir(cleaned).filePath("dblp.xml"));
    if (guessedFile.exists()) {
        return cleaned;
    }
    return QString();
}

QString AnalyticsService::authorResultFilePath() const
{
    return QDir(m_baseDirectory).filePath("analysis/f3_top_authors.txt");
}

QString AnalyticsService::hotspotResultFilePath() const
{
    return QDir(m_baseDirectory).filePath("analysis/f4_yearly_hotspots.txt");
}

bool AnalyticsService::tryLoadFromMemoryCache()
{
    const auto it = analyticsCache().constFind(m_baseDirectory);
    if (it == analyticsCache().constEnd()) {
        return false;
    }
    if (!it->valid || it->sourceVersion != sourceVersionStamp()) {
        return false;
    }

    m_topAuthors = it->topAuthors;
    m_hotKeywordsByYear = it->hotKeywordsByYear;
    return true;
}

bool AnalyticsService::tryLoadFromResultFiles()
{
    QFileInfo authorInfo(authorResultFilePath());
    QFileInfo hotspotInfo(hotspotResultFilePath());
    if (!authorInfo.exists() || !hotspotInfo.exists()) {
        return false;
    }

    const QString currentVersion = sourceVersionStamp();
    if (authorInfo.lastModified().toString(Qt::ISODateWithMs) < currentVersion || hotspotInfo.lastModified().toString(Qt::ISODateWithMs) < currentVersion) {
        return false;
    }

    QVector<AuthorStat> loadedAuthors;
    QMap<int, QVector<KeywordStat>> loadedHotspots;

    QFile authorFile(authorResultFilePath());
    if (!authorFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream authorStream(&authorFile);
    bool skipHeader = true;
    while (!authorStream.atEnd()) {
        const QString line = safeLineRead(authorStream).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (skipHeader) {
            skipHeader = false;
            continue;
        }
        const QStringList parts = line.split('\t');
        if (parts.size() < 3) {
            continue;
        }
        bool ok = false;
        const int paperCount = parts[2].toInt(&ok);
        if (!ok || parts[1].trimmed().isEmpty()) {
            continue;
        }
        loadedAuthors.push_back({parts[1].trimmed(), paperCount});
    }
    authorFile.close();

    QFile hotspotFile(hotspotResultFilePath());
    if (!hotspotFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream hotspotStream(&hotspotFile);
    int currentYear = 0;
    while (!hotspotStream.atEnd()) {
        const QString line = safeLineRead(hotspotStream).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith("Year ")) {
            bool ok = false;
            currentYear = line.mid(5).toInt(&ok);
            if (!ok) {
                currentYear = 0;
            }
            continue;
        }
        if (currentYear == 0) {
            continue;
        }

        const int dotIndex = line.indexOf(". ");
        const int leftParen = line.lastIndexOf(" (");
        const int rightParen = line.lastIndexOf(')');
        if (dotIndex < 0 || leftParen < 0 || rightParen <= leftParen) {
            continue;
        }

        const QString word = line.mid(dotIndex + 2, leftParen - dotIndex - 2).trimmed();
        bool ok = false;
        const int frequency = line.mid(leftParen + 2, rightParen - leftParen - 2).toInt(&ok);
        if (!ok || word.isEmpty()) {
            continue;
        }
        loadedHotspots[currentYear].push_back({word, frequency});
    }
    hotspotFile.close();

    if (loadedAuthors.isEmpty() || loadedHotspots.isEmpty()) {
        return false;
    }

    m_topAuthors = loadedAuthors;
    m_hotKeywordsByYear = loadedHotspots;
    return true;
}

void AnalyticsService::updateMemoryCache() const
{
    AnalyticsCacheEntry entry;
    entry.topAuthors = m_topAuthors;
    entry.hotKeywordsByYear = m_hotKeywordsByYear;
    entry.sourceVersion = sourceVersionStamp();
    entry.valid = true;
    analyticsCache().insert(m_baseDirectory, entry);
}

void AnalyticsService::loadStopWords()
{
    if (m_stopWordsReady) {
        return;
    }

    const QStringList stopWords = {
        "a", "an", "and", "are", "as", "at", "be", "based", "by", "for", "from",
        "in", "into", "is", "of", "on", "or", "over", "the", "their", "to", "toward",
        "towards", "under", "using", "via", "with", "without", "within", "study",
        "analysis", "approach", "method", "methods", "system", "systems", "model",
        "models", "new", "through", "paper"
    };
    for (const QString& word : stopWords) {
        m_stopWords.insert(word);
    }
    m_stopWordsReady = true;
}

QStringList AnalyticsService::tokenizeTitle(const QString& title) const
{
    QString normalized = title.toLower();
    normalized.replace(QRegularExpression("[^a-z0-9]+"), " ");
    const QStringList rawTokens = normalized.split(' ', Qt::SkipEmptyParts);

    QStringList tokens;
    tokens.reserve(rawTokens.size());
    for (QString token : rawTokens) {
        if (token.size() <= 1) {
            continue;
        }
        if (token.endsWith("ies") && token.size() > 4) {
            token.chop(3);
            token.append('y');
        } else if (token.endsWith("es") && token.size() > 3) {
            token.chop(2);
        } else if (token.endsWith('s') && token.size() > 3) {
            token.chop(1);
        }
        if (m_stopWords.contains(token)) {
            continue;
        }
        tokens.push_back(token);
    }
    return tokens;
}

void AnalyticsService::loadAuthorStats()
{
    loadAuthorCountsFromRankFile();
    if (!m_lastError.isEmpty()) {
        return;
    }

    BinaryHeap<AuthorStat, decltype(&authorHeapLess)> heap(authorHeapLess);
    const QVector<QPair<QString, int>> orderedAuthors = m_authorCounts.inOrder();
    for (const auto& entry : orderedAuthors) {
        heap.push({entry.first, entry.second});
        if (heap.size() > 100) {
            heap.pop();
        }
    }

    m_topAuthors = heap.values();
    std::sort(m_topAuthors.begin(), m_topAuthors.end(), authorOutputGreater);
}

void AnalyticsService::loadHotspotStats()
{
    const QDir yearDir(QDir(m_baseDirectory).filePath("database/year"));
    if (!yearDir.exists()) {
        m_lastError = QString::fromUtf8("未找到年份索引目录：") + yearDir.absolutePath();
        return;
    }

    const QStringList yearFiles = yearDir.entryList(QStringList() << "*.ini", QDir::Files, QDir::Name);
    for (const QString& fileName : yearFiles) {
        bool ok = false;
        const int year = fileName.left(fileName.indexOf('.')).toInt(&ok);
        if (!ok || year <= 0) {
            continue;
        }

        QFile file(yearDir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        TrieTree trie;
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            const QString title = safeLineRead(stream).trimmed();
            if (title.isEmpty()) {
                continue;
            }
            const QStringList tokens = tokenizeTitle(title);
            for (const QString& token : tokens) {
                trie.insert(token);
            }
        }
        file.close();

        QVector<KeywordStat> keywords;
        trie.collect(keywords);
        BinaryHeap<KeywordStat, decltype(&keywordHeapLess)> heap(keywordHeapLess);
        for (const KeywordStat& keyword : keywords) {
            heap.push(keyword);
            if (heap.size() > 10) {
                heap.pop();
            }
        }

        QVector<KeywordStat> yearlyTop = heap.values();
        std::sort(yearlyTop.begin(), yearlyTop.end(), keywordOutputGreater);
        m_hotKeywordsByYear.insert(year, yearlyTop);
    }
}

void AnalyticsService::writeAuthorResultFile(const QVector<AuthorStat>& authors) const
{
    QDir baseDir(m_baseDirectory);
    baseDir.mkpath("analysis");
    QFile file(baseDir.filePath("analysis/f3_top_authors.txt"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << "Rank\tAuthor\tPaperCount\n";
    for (int index = 0; index < authors.size(); ++index) {
        stream << index + 1 << '\t' << authors[index].name << '\t' << authors[index].paperCount << '\n';
    }
}

void AnalyticsService::writeHotspotResultFile() const
{
    QDir baseDir(m_baseDirectory);
    baseDir.mkpath("analysis");
    QFile file(baseDir.filePath("analysis/f4_yearly_hotspots.txt"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    for (auto it = m_hotKeywordsByYear.cbegin(); it != m_hotKeywordsByYear.cend(); ++it) {
        stream << "Year " << it.key() << '\n';
        const QVector<KeywordStat>& keywords = it.value();
        for (int index = 0; index < keywords.size(); ++index) {
            stream << "  " << index + 1 << ". " << keywords[index].word << " (" << keywords[index].frequency << ")\n";
        }
        stream << '\n';
    }
}

void AnalyticsService::loadAuthorCountsFromRankFile()
{
    m_authorCounts = AvlMap<QString, int>();
    const QString authorRankPath = QDir(m_baseDirectory).filePath("database/author_rank");
    QFile file(authorRankPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString::fromUtf8("无法读取作者统计文件：") + authorRankPath;
        return;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = safeLineRead(stream).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const int blankIndex = line.indexOf(' ');
        if (blankIndex <= 0) {
            continue;
        }

        bool ok = false;
        const int count = line.left(blankIndex).toInt(&ok);
        const QString authorName = line.mid(blankIndex + 1).trimmed();
        if (!ok || authorName.isEmpty()) {
            continue;
        }
        m_authorCounts.insertOrAssign(authorName, count);
    }
    file.close();
}

void AnalyticsService::rebuildAuthorIndexFromTopAuthors()
{
    m_authorCounts = AvlMap<QString, int>();
    for (const AuthorStat& author : m_topAuthors) {
        m_authorCounts.insertOrAssign(author.name, author.paperCount);
    }
}

QString AnalyticsService::sourceVersionStamp() const
{
    const QFileInfo authorRankInfo(QDir(m_baseDirectory).filePath("database/author_rank"));
    const QFileInfo finishInfo(QDir(m_baseDirectory).filePath("database/finish.db"));
    const QString left = authorRankInfo.lastModified().toString(Qt::ISODateWithMs);
    const QString right = finishInfo.lastModified().toString(Qt::ISODateWithMs);
    return (left > right) ? left : right;
}
