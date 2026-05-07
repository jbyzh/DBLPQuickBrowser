#ifndef SEARCH_H
#define SEARCH_H

#include <QString>
#include <QVector>

namespace search_module {

struct SearchResult
{
    QString title;
    QString authors;
    QString year;
    QString venue;
    QString journal;
    QString volume;
    QString number;
    QString pages;
    QString doi;
    qint64 offset = -1;
    bool detailsLoaded = false;
};

struct YearDistribution
{
    int year = 0;
    int count = 0;
};

class StringMatcher
{
public:
    static QVector<int> kmpSearch(const QString& text, const QString& pattern);

private:
    static QVector<int> computeNext(const QString& pattern);
};

class Searcher
{
public:
    explicit Searcher(const QString& xmlPath = QString());

    void setXmlPath(const QString& xmlPath);
    QString databasePath() const;
    QString lastError() const;

    QVector<SearchResult> searchByAuthor(const QString& authorName) const;
    QVector<SearchResult> searchByTitle(const QString& title) const;
    QVector<SearchResult> fuzzySearch(const QString& keyword) const;
    QVector<YearDistribution> getYearDistribution(const QVector<SearchResult>& results) const;
    QString highlightKeyword(const QString& text, const QString& keyword) const;
    void readPaperDetails(SearchResult& result) const;

private:
    QString normalizeBaseDirectory(const QString& rawPath) const;
    QString xmlFilePath() const;
    quint32 hash4(const QString& text) const;
    QVector<QString> readLines(const QString& filePath) const;
    SearchResult parseArticleLine(const QString& line) const;
    SearchResult parseAuthorLine(const QString& line) const;
    void parsePaperXml(const QString& xml, SearchResult& result) const;
    QString extractSingleTag(const QString& xml, const QString& tag) const;
    QStringList extractMultiTag(const QString& xml, const QString& tag) const;
    int parseYearValue(const QString& yearText) const;

    QString m_xmlPath;
    QString m_baseDirectory;
    mutable QString m_lastError;
};

} // namespace search_module

#endif
