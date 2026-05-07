#include "authorindexgraphcache.h"
#include "cooperationnet.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

namespace {

QMutex g_mtx;
QString g_canonKey;
QHash<QString, QHash<QString, int>> g_adj;
bool g_valid = false;

QString g_prCanonKey;
QHash<QString, double> g_prByAuthor;
bool g_prValid = false;

} // namespace

QString AuthorIndexGraphCache::canonicalAuthorDir(const QString& dirPath)
{
    const QString abs = QDir(dirPath).absolutePath();
    const QString c = QFileInfo(abs).canonicalFilePath();
    return c.isEmpty() ? abs : c;
}

bool AuthorIndexGraphCache::tryReuse(const QString& candidateDir, CooperationNet* net)
{
    if (!net) {
        return false;
    }
    const QString key = canonicalAuthorDir(candidateDir);
    if (key.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&g_mtx);
    if (!g_valid || key != g_canonKey) {
        return false;
    }
    net->replaceAdjacencyData(g_adj);
    return true;
}

void AuthorIndexGraphCache::remember(const QString& loadedDirUsed, const QHash<QString, QHash<QString, int>>& adj)
{
    QMutexLocker locker(&g_mtx);
    g_canonKey = canonicalAuthorDir(loadedDirUsed);
    g_adj = adj;
    g_valid = !g_adj.isEmpty();
    g_prValid = false;
}

bool AuthorIndexGraphCache::tryReusePageRank(const QString& graphCanonKey, QHash<QString, double>* outPrByAuthor)
{
    if (!outPrByAuthor || graphCanonKey.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&g_mtx);
    if (!g_prValid || graphCanonKey != g_prCanonKey) {
        return false;
    }
    *outPrByAuthor = g_prByAuthor;
    return true;
}

void AuthorIndexGraphCache::rememberPageRank(const QString& graphCanonKey, const QHash<QString, double>& prByAuthor)
{
    QMutexLocker locker(&g_mtx);
    g_prCanonKey = graphCanonKey;
    g_prByAuthor = prByAuthor;
    g_prValid = !g_prByAuthor.isEmpty();
}
