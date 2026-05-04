#ifndef INTERVALANALYSIS_H
#define INTERVALANALYSIS_H

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

#include "analytics.h"

struct IntervalSummary
{
    int totalPapers = 0;
    int averagePapers = 0;
    int peakYear = 0;
    int peakCount = 0;
};

class YearSegmentTree
{
public:
    void build(const QVector<int>& counts);
    int querySum(int left, int right) const;
    QPair<int, int> queryPeak(int left, int right) const;

private:
    struct Node {
        int left = 0;
        int right = 0;
        int sum = 0;
        int peakCount = 0;
        int peakIndex = 0;
    };

    void buildNode(int index, int left, int right, const QVector<int>& counts);
    int querySumNode(int index, int left, int right) const;
    Node queryPeakNode(int index, int left, int right) const;

    QVector<Node> m_tree;
    int m_size = 0;
};

class IntervalAnalysisService
{
public:
    explicit IntervalAnalysisService(const QString& xmlPath = QString());

    void setXmlPath(const QString& xmlPath);
    bool ensureLoaded();
    QString lastError() const;

    QList<int> availableYears() const;
    IntervalSummary querySummary(int startYear, int endYear) const;
    QVector<QPair<int, int>> querySeries(int startYear, int endYear) const;
    QVector<KeywordStat> queryHotKeywords(int startYear, int endYear, int topN = 10) const;
    QVector<QPair<int, int>> queryKeywordTrend(const QString& keyword, int startYear, int endYear) const;

private:
    QString normalizeBaseDirectory(const QString& rawPath) const;
    QString cacheFilePath() const;
    QString sourceVersionStamp() const;
    bool tryLoadFromMemoryCache();
    bool tryLoadFromResultFile();
    void updateMemoryCache() const;
    void writeCacheFile() const;
    void loadStopWords();
    QStringList tokenizeTitle(const QString& title) const;
    bool buildYearIndex();
    int yearToIndex(int year) const;

    QString m_xmlPath;
    QString m_baseDirectory;
    QString m_lastError;
    bool m_loaded = false;
    bool m_stopWordsReady = false;

    QVector<int> m_years;
    QVector<int> m_yearCounts;
    QMap<int, QHash<QString, int>> m_keywordByYear;
    QSet<QString> m_stopWords;
    YearSegmentTree m_segmentTree;
};

#endif
