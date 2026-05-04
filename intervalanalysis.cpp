#include "intervalanalysis.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace {

struct IntervalCacheEntry
{
    QVector<int> years;
    QVector<int> yearCounts;
    QMap<int, QHash<QString, int>> keywordByYear;
    QString sourceVersion;
    bool valid = false;
};

QHash<QString, IntervalCacheEntry>& intervalCache()
{
    static QHash<QString, IntervalCacheEntry> cache;
    return cache;
}

QString safeIntervalLineRead(QTextStream& stream)
{
    QString line = stream.readLine();
    while (!line.isNull() && line.endsWith('\r')) {
        line.chop(1);
    }
    return line;
}

bool keywordHeapLessForInterval(const KeywordStat& left, const KeywordStat& right)
{
    if (left.frequency != right.frequency) {
        return left.frequency < right.frequency;
    }
    return left.word > right.word;
}

bool keywordOutputGreaterForInterval(const KeywordStat& left, const KeywordStat& right)
{
    if (left.frequency != right.frequency) {
        return left.frequency > right.frequency;
    }
    return left.word < right.word;
}

}

void YearSegmentTree::build(const QVector<int>& counts)
{
    m_size = counts.size();
    if (m_size == 0) {
        m_tree.clear();
        return;
    }
    m_tree.resize(m_size * 4);
    buildNode(1, 0, m_size - 1, counts);
}

int YearSegmentTree::querySum(int left, int right) const
{
    if (m_size == 0 || left > right) {
        return 0;
    }
    return querySumNode(1, left, right);
}

QPair<int, int> YearSegmentTree::queryPeak(int left, int right) const
{
    if (m_size == 0 || left > right) {
        return qMakePair(-1, 0);
    }
    const Node result = queryPeakNode(1, left, right);
    return qMakePair(result.peakIndex, result.peakCount);
}

void YearSegmentTree::buildNode(int index, int left, int right, const QVector<int>& counts)
{
    Node& node = m_tree[index];
    node.left = left;
    node.right = right;
    if (left == right) {
        node.sum = counts[left];
        node.peakCount = counts[left];
        node.peakIndex = left;
        return;
    }

    const int mid = (left + right) / 2;
    buildNode(index * 2, left, mid, counts);
    buildNode(index * 2 + 1, mid + 1, right, counts);

    const Node& lhs = m_tree[index * 2];
    const Node& rhs = m_tree[index * 2 + 1];
    node.sum = lhs.sum + rhs.sum;
    if (lhs.peakCount >= rhs.peakCount) {
        node.peakCount = lhs.peakCount;
        node.peakIndex = lhs.peakIndex;
    } else {
        node.peakCount = rhs.peakCount;
        node.peakIndex = rhs.peakIndex;
    }
}

int YearSegmentTree::querySumNode(int index, int left, int right) const
{
    const Node& node = m_tree[index];
    if (left <= node.left && node.right <= right) {
        return node.sum;
    }

    const int mid = (node.left + node.right) / 2;
    int result = 0;
    if (left <= mid) {
        result += querySumNode(index * 2, left, right);
    }
    if (right > mid) {
        result += querySumNode(index * 2 + 1, left, right);
    }
    return result;
}

YearSegmentTree::Node YearSegmentTree::queryPeakNode(int index, int left, int right) const
{
    const Node& node = m_tree[index];
    if (left <= node.left && node.right <= right) {
        return node;
    }

    const int mid = (node.left + node.right) / 2;
    if (right <= mid) {
        return queryPeakNode(index * 2, left, right);
    }
    if (left > mid) {
        return queryPeakNode(index * 2 + 1, left, right);
    }

    const Node lhs = queryPeakNode(index * 2, left, right);
    const Node rhs = queryPeakNode(index * 2 + 1, left, right);
    Node merged;
    if (lhs.peakCount >= rhs.peakCount) {
        merged.peakCount = lhs.peakCount;
        merged.peakIndex = lhs.peakIndex;
    } else {
        merged.peakCount = rhs.peakCount;
        merged.peakIndex = rhs.peakIndex;
    }
    return merged;
}

IntervalAnalysisService::IntervalAnalysisService(const QString& xmlPath)
{
    setXmlPath(xmlPath);
}

void IntervalAnalysisService::setXmlPath(const QString& xmlPath)
{
    m_xmlPath = xmlPath;
    m_baseDirectory = normalizeBaseDirectory(xmlPath);
    m_loaded = false;
}

bool IntervalAnalysisService::ensureLoaded()
{
    if (m_loaded) {
        return true;
    }

    m_lastError.clear();
    if (m_baseDirectory.isEmpty()) {
        m_lastError = QString::fromUtf8("未找到有效的数据目录。");
        return false;
    }

    const QFileInfo finishFile(QDir(m_baseDirectory).filePath("database/finish.db"));
    if (!finishFile.exists()) {
        m_lastError = QString::fromUtf8("请先在首页完成 dblp.xml 解析。");
        return false;
    }

    loadStopWords();
    if (tryLoadFromMemoryCache() || tryLoadFromResultFile()) {
        m_segmentTree.build(m_yearCounts);
        m_loaded = true;
        return true;
    }

    if (!buildYearIndex()) {
        return false;
    }

    writeCacheFile();
    updateMemoryCache();
    m_segmentTree.build(m_yearCounts);
    m_loaded = true;
    return true;
}

QString IntervalAnalysisService::lastError() const
{
    return m_lastError;
}

QList<int> IntervalAnalysisService::availableYears() const
{
    return QList<int>(m_years.begin(), m_years.end());
}

IntervalSummary IntervalAnalysisService::querySummary(int startYear, int endYear) const
{
    IntervalSummary summary;
    const int left = yearToIndex(startYear);
    const int right = yearToIndex(endYear);
    if (left < 0 || right < 0 || left > right) {
        return summary;
    }

    summary.totalPapers = m_segmentTree.querySum(left, right);
    summary.averagePapers = (right - left + 1) > 0 ? summary.totalPapers / (right - left + 1) : 0;
    const QPair<int, int> peak = m_segmentTree.queryPeak(left, right);
    if (peak.first >= 0 && peak.first < m_years.size()) {
        summary.peakYear = m_years[peak.first];
        summary.peakCount = peak.second;
    }
    return summary;
}

QVector<QPair<int, int>> IntervalAnalysisService::querySeries(int startYear, int endYear) const
{
    QVector<QPair<int, int>> series;
    const int left = yearToIndex(startYear);
    const int right = yearToIndex(endYear);
    if (left < 0 || right < 0 || left > right) {
        return series;
    }

    for (int index = left; index <= right; ++index) {
        series.push_back(qMakePair(m_years[index], m_yearCounts[index]));
    }
    return series;
}

QVector<KeywordStat> IntervalAnalysisService::queryHotKeywords(int startYear, int endYear, int topN) const
{
    QVector<KeywordStat> result;
    const int left = yearToIndex(startYear);
    const int right = yearToIndex(endYear);
    if (left < 0 || right < 0 || left > right || topN <= 0) {
        return result;
    }

    QHash<QString, int> merged;
    for (int index = left; index <= right; ++index) {
        const auto yearIt = m_keywordByYear.constFind(m_years[index]);
        if (yearIt == m_keywordByYear.constEnd()) {
            continue;
        }
        for (auto it = yearIt->cbegin(); it != yearIt->cend(); ++it) {
            merged[it.key()] += it.value();
        }
    }

    BinaryHeap<KeywordStat, decltype(&keywordHeapLessForInterval)> heap(keywordHeapLessForInterval);
    for (auto it = merged.cbegin(); it != merged.cend(); ++it) {
        heap.push({it.key(), it.value()});
        if (heap.size() > topN) {
            heap.pop();
        }
    }

    result = heap.values();
    std::sort(result.begin(), result.end(), keywordOutputGreaterForInterval);
    return result;
}

QVector<QPair<int, int>> IntervalAnalysisService::queryKeywordTrend(const QString& keyword, int startYear, int endYear) const
{
    QVector<QPair<int, int>> result;
    const QString normalized = keyword.trimmed().toLower();
    const int left = yearToIndex(startYear);
    const int right = yearToIndex(endYear);
    if (normalized.isEmpty() || left < 0 || right < 0 || left > right) {
        return result;
    }

    for (int index = left; index <= right; ++index) {
        const int year = m_years[index];
        const int value = m_keywordByYear.value(year).value(normalized, 0);
        result.push_back(qMakePair(year, value));
    }
    return result;
}

QString IntervalAnalysisService::normalizeBaseDirectory(const QString& rawPath) const
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

QString IntervalAnalysisService::cacheFilePath() const
{
    return QDir(m_baseDirectory).filePath("analysis/f8_interval_cache.txt");
}

QString IntervalAnalysisService::sourceVersionStamp() const
{
    const QFileInfo finishInfo(QDir(m_baseDirectory).filePath("database/finish.db"));
    return finishInfo.lastModified().toString(Qt::ISODateWithMs);
}

bool IntervalAnalysisService::tryLoadFromMemoryCache()
{
    const auto it = intervalCache().constFind(m_baseDirectory);
    if (it == intervalCache().constEnd()) {
        return false;
    }
    if (!it->valid || it->sourceVersion != sourceVersionStamp()) {
        return false;
    }

    m_years = it->years;
    m_yearCounts = it->yearCounts;
    m_keywordByYear = it->keywordByYear;
    return !m_years.isEmpty();
}

bool IntervalAnalysisService::tryLoadFromResultFile()
{
    const QFileInfo cacheInfo(cacheFilePath());
    if (!cacheInfo.exists() || cacheInfo.lastModified().toString(Qt::ISODateWithMs) < sourceVersionStamp()) {
        return false;
    }

    QFile file(cacheFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QVector<int> loadedYears;
    QVector<int> loadedCounts;
    QMap<int, QHash<QString, int>> loadedKeywords;
    QTextStream stream(&file);
    int currentYear = 0;

    while (!stream.atEnd()) {
        const QString line = safeIntervalLineRead(stream).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith("VERSION\t")) {
            continue;
        }
        if (line.startsWith("YEAR\t")) {
            const QStringList parts = line.split('\t');
            if (parts.size() < 3) {
                currentYear = 0;
                continue;
            }
            bool yearOk = false;
            bool countOk = false;
            const int year = parts[1].toInt(&yearOk);
            const int count = parts[2].toInt(&countOk);
            if (!yearOk || !countOk) {
                currentYear = 0;
                continue;
            }
            currentYear = year;
            loadedYears.push_back(year);
            loadedCounts.push_back(count);
            loadedKeywords.insert(year, {});
            continue;
        }
        if (line == "ENDYEAR") {
            currentYear = 0;
            continue;
        }
        if (currentYear == 0) {
            continue;
        }

        const int splitIndex = line.lastIndexOf('\t');
        if (splitIndex <= 0) {
            continue;
        }
        bool freqOk = false;
        const int frequency = line.mid(splitIndex + 1).toInt(&freqOk);
        const QString word = line.left(splitIndex).trimmed();
        if (!freqOk || word.isEmpty()) {
            continue;
        }
        loadedKeywords[currentYear].insert(word, frequency);
    }

    file.close();
    if (loadedYears.isEmpty()) {
        return false;
    }

    m_years = loadedYears;
    m_yearCounts = loadedCounts;
    m_keywordByYear = loadedKeywords;
    updateMemoryCache();
    return true;
}

void IntervalAnalysisService::updateMemoryCache() const
{
    IntervalCacheEntry entry;
    entry.years = m_years;
    entry.yearCounts = m_yearCounts;
    entry.keywordByYear = m_keywordByYear;
    entry.sourceVersion = sourceVersionStamp();
    entry.valid = true;
    intervalCache().insert(m_baseDirectory, entry);
}

void IntervalAnalysisService::writeCacheFile() const
{
    QDir baseDir(m_baseDirectory);
    baseDir.mkpath("analysis");
    QFile file(cacheFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << "VERSION\t" << sourceVersionStamp() << '\n';
    for (int index = 0; index < m_years.size(); ++index) {
        const int year = m_years[index];
        stream << "YEAR\t" << year << '\t' << m_yearCounts[index] << '\n';
        const QHash<QString, int>& keywords = m_keywordByYear.value(year);
        for (auto it = keywords.cbegin(); it != keywords.cend(); ++it) {
            stream << it.key() << '\t' << it.value() << '\n';
        }
        stream << "ENDYEAR\n";
    }
}

void IntervalAnalysisService::loadStopWords()
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

QStringList IntervalAnalysisService::tokenizeTitle(const QString& title) const
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

bool IntervalAnalysisService::buildYearIndex()
{
    m_years.clear();
    m_yearCounts.clear();
    m_keywordByYear.clear();

    const QDir yearDir(QDir(m_baseDirectory).filePath("database/year"));
    if (!yearDir.exists()) {
        m_lastError = QString::fromUtf8("未找到年份索引目录：") + yearDir.absolutePath();
        return false;
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

        int count = 0;
        QHash<QString, int> keywordMap;
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            const QString title = safeIntervalLineRead(stream).trimmed();
            if (title.isEmpty()) {
                continue;
            }
            ++count;
            const QStringList tokens = tokenizeTitle(title);
            for (const QString& token : tokens) {
                ++keywordMap[token];
            }
        }
        file.close();

        m_years.push_back(year);
        m_yearCounts.push_back(count);
        m_keywordByYear.insert(year, keywordMap);
    }

    if (m_years.isEmpty()) {
        m_lastError = QString::fromUtf8("未找到可分析的年份数据。");
        return false;
    }

    return true;
}

int IntervalAnalysisService::yearToIndex(int year) const
{
    for (int index = 0; index < m_years.size(); ++index) {
        if (m_years[index] == year) {
            return index;
        }
    }
    return -1;
}
