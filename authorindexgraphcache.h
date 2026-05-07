#ifndef AUTHORINDEXGRAPHCACHE_H
#define AUTHORINDEXGRAPHCACHE_H

#include <QHash>
#include <QString>

class CooperationNet;

/**
 * 进程内缓存已成功加载的 author 索引合作图，避免每开一个 Precise 子窗口都重新读盘解析。
 */
namespace AuthorIndexGraphCache {
QString canonicalAuthorDir(const QString& dirPath);
bool tryReuse(const QString& candidateDir, CooperationNet* net);
void remember(const QString& loadedDirUsed, const QHash<QString, QHash<QString, int>>& adj);

/** 与当前 author 目录 canonical 键一致的 PageRank 结果复用（避免重复 QtConcurrent 计算）。 */
bool tryReusePageRank(const QString& graphCanonKey, QHash<QString, double>* outPrByAuthor);
void rememberPageRank(const QString& graphCanonKey, const QHash<QString, double>& prByAuthor);
} // namespace AuthorIndexGraphCache

#endif
