#ifndef COOPERATIONNET_H
#define COOPERATIONNET_H

#include <QWidget>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QString>
#include <QPair>

class CooperationNet : public QWidget
{
    Q_OBJECT
public:
    // 构造函数,创建一个合作网络对象
    explicit CooperationNet(QWidget* parent = nullptr);
    // 数据管理（增删加载）
    void addPublication(const QStringList& authors, const QString& title);
    void clear();
    // 从文件夹加载数据
    bool loadFromAuthorIndexDir(const QString& authorIndexDirPath);
    // 合作者查询
    QHash<QString, int> getCollaborators(const QString& name);
    // 推荐与权威排名
    QVector<QPair<QString, int>> recommendAuthors(const QString& name, int topN = 10) const;
    QVector<QPair<QString, double>> pageRank(int iterations = 20, double damping = 0.85) const;
    static QVector<QPair<QString, double>> computePageRank(const QHash<QString, QHash<QString, int>>& adjList,
        int iterations = 20,
        double damping = 0.85);
    // 工具函数
    QStringList allAuthors() const;
    bool hasAuthor(const QString& name) const;
    int authorCount() const;
    QHash<QString, QHash<QString, int>> adjacencyCopy() const;
    // 用已有合作图替换内存图（用于跨窗口缓存命中，避免再次读盘）
    void replaceAdjacencyData(const QHash<QString, QHash<QString, int>>& adj);

private:
    QHash<QString, QHash<QString, int>> m_adjList; // 作者合作图
};

#endif
