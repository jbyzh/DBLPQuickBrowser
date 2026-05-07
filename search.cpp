#include "search.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

#include <fstream>

namespace search_module {

namespace {

QString safeLineRead(QTextStream& stream)
{
    QString line = stream.readLine();
    while (!line.isNull() && line.endsWith('\r')) {
        line.chop(1);
    }
    return line;
}

QString decodeXmlEntities(QString text)
{
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&amp;", "&");
    text.replace("&quot;", "\"");
    text.replace("&apos;", "'");
    return text.trimmed();
}

QString detectRecordTag(const QString& firstLine)
{
    static const QStringList tags = {
        "article", "inproceedings", "proceedings", "book",
        "incollection", "phdthesis", "mastersthesis", "www",
        "person", "data"
    };

    for (const QString& tag : tags) {
        if (firstLine.contains("<" + tag, Qt::CaseInsensitive)) {
            return tag;
        }
    }
    return QString();
}

}

Searcher::Searcher(const QString& xmlPath)
{
    setXmlPath(xmlPath);
}

void Searcher::setXmlPath(const QString& xmlPath)
{
    m_xmlPath = xmlPath;
    m_baseDirectory = normalizeBaseDirectory(xmlPath);
    m_lastError.clear();
}

QString Searcher::databasePath() const
{
    return QDir(m_baseDirectory).filePath("database");
}

QString Searcher::lastError() const
{
    return m_lastError;
}

QVector<SearchResult> Searcher::searchByAuthor(const QString& authorName) const
{
    QVector<SearchResult> results;
    const QString keyword = authorName.trimmed();
    if (keyword.isEmpty()) {
        return results;
    }

    const QString filePath = QDir(databasePath()).filePath(
        "author/" + QString::number(hash4(keyword)) + ".ini");
    const QVector<QString> lines = readLines(filePath);
    for (const QString& line : lines) {
        SearchResult result = parseAuthorLine(line);
        if (result.title.isEmpty() || result.authors.isEmpty()) {
            continue;
        }
        if (result.authors.contains(keyword, Qt::CaseInsensitive)) {
            results.push_back(result);
        }
    }
    return results;
}

QVector<SearchResult> Searcher::searchByTitle(const QString& title) const
{
    QVector<SearchResult> results;
    const QString keyword = title.trimmed();
    if (keyword.isEmpty()) {
        return results;
    }

    const QString filePath = QDir(databasePath()).filePath(
        "article/" + QString::number(hash4(keyword)) + ".ini");
    const QVector<QString> lines = readLines(filePath);
    for (const QString& line : lines) {
        SearchResult result = parseArticleLine(line);
        if (result.title.compare(keyword, Qt::CaseInsensitive) == 0) {
            results.push_back(result);
        }
    }
    return results;
}

QVector<SearchResult> Searcher::fuzzySearch(const QString& keyword) const
{
    QVector<SearchResult> results;
    const QString loweredKeyword = keyword.trimmed().toLower();
    if (loweredKeyword.isEmpty()) {
        return results;
    }

    QDir articleDir(QDir(databasePath()).filePath("article"));
    const QStringList files = articleDir.entryList(QStringList() << "*.ini", QDir::Files, QDir::Name);
    for (const QString& fileName : files) {
        const QVector<QString> lines = readLines(articleDir.filePath(fileName));
        for (const QString& line : lines) {
            SearchResult result = parseArticleLine(line);
            if (result.title.isEmpty()) {
                continue;
            }
            if (!StringMatcher::kmpSearch(result.title.toLower(), loweredKeyword).isEmpty()) {
                results.push_back(result);
            }
        }
    }
    return results;
}

QVector<YearDistribution> Searcher::getYearDistribution(const QVector<SearchResult>& results) const
{
    QMap<int, int> counter;
    for (const SearchResult& result : results) {
        const int year = parseYearValue(result.year);
        if (year > 0) {
            counter[year] += 1;
        }
    }

    QVector<YearDistribution> distribution;
    distribution.reserve(counter.size());
    for (auto it = counter.cbegin(); it != counter.cend(); ++it) {
        distribution.push_back({it.key(), it.value()});
    }
    return distribution;
}

QString Searcher::highlightKeyword(const QString& text, const QString& keyword) const
{
    const QString trimmedKeyword = keyword.trimmed();
    if (trimmedKeyword.isEmpty()) {
        return text;
    }

    QString result = text;
    QRegularExpression regex(QRegularExpression::escape(trimmedKeyword),
                             QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator matches = regex.globalMatch(result);
    int offset = 0;
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        const int position = match.capturedStart() + offset;
        const int length = match.capturedLength();
        const QString replacement = QString("<span style=\"color:#F56C6C;font-weight:700;\">%1</span>")
                                        .arg(match.captured());
        result.replace(position, length, replacement);
        offset += replacement.size() - length;
    }
    return result;
}

void Searcher::readPaperDetails(SearchResult& result) const
{
    if (result.detailsLoaded || result.offset < 0) {
        result.detailsLoaded = true;
        return;
    }

    const QString fullXmlPath = xmlFilePath();
    QFileInfo xmlInfo(fullXmlPath);
    if (!xmlInfo.exists()) {
        m_lastError = QString::fromUtf8("未找到 dblp.xml 文件。");
        return;
    }

    std::ifstream xmlFile(fullXmlPath.toStdString(), std::ios::binary);
    if (!xmlFile.is_open()) {
        m_lastError = QString::fromUtf8("无法打开 dblp.xml 文件。");
        return;
    }

    xmlFile.seekg(static_cast<std::streamoff>(result.offset));
    std::string lineStd;
    QString recordXml;
    QString recordTag;
    bool inPaper = false;
    int tagCount = 0;

    while (std::getline(xmlFile, lineStd)) {
        const QString line = QString::fromStdString(lineStd);
        const QString detected = detectRecordTag(line);
        if (!detected.isEmpty()) {
            inPaper = true;
            recordTag = detected;
            tagCount = 1;
            recordXml += line + "\n";
            break;
        }
    }

    if (!inPaper || recordTag.isEmpty()) {
        return;
    }

    while (std::getline(xmlFile, lineStd)) {
        const QString line = QString::fromStdString(lineStd);
        recordXml += line + "\n";

        int startTagCount = 0;
        int endTagCount = 0;

        QRegularExpression openTagRe("<([A-Za-z][^/>\\s]*)[^>]*>");
        QRegularExpression closeTagRe("</([A-Za-z][^>\\s]*)>");

        QRegularExpressionMatchIterator openIt = openTagRe.globalMatch(line);
        while (openIt.hasNext()) {
            const auto match = openIt.next();
            if (!match.captured(0).endsWith("/>")) {
                ++startTagCount;
            }
        }

        QRegularExpressionMatchIterator closeIt = closeTagRe.globalMatch(line);
        while (closeIt.hasNext()) {
            closeIt.next();
            ++endTagCount;
        }

        tagCount += startTagCount;
        tagCount -= endTagCount;

        if (line.contains("</" + recordTag + ">", Qt::CaseInsensitive) || tagCount <= 0) {
            break;
        }
    }
    xmlFile.close();

    parsePaperXml(recordXml, result);
    result.detailsLoaded = true;
}

QString Searcher::normalizeBaseDirectory(const QString& rawPath) const
{
    if (rawPath.trimmed().isEmpty()) {
        return QString();
    }

    QFileInfo info(rawPath);
    if (info.isFile()) {
        return QDir::cleanPath(info.absolutePath());
    }
    if (info.isDir()) {
        QFileInfo xmlInfo(QDir(info.absoluteFilePath()).filePath("dblp.xml"));
        if (xmlInfo.exists()) {
            return QDir::cleanPath(info.absoluteFilePath());
        }
        return QDir::cleanPath(info.absoluteFilePath());
    }

    return QString();
}

QString Searcher::xmlFilePath() const
{
    QFileInfo rawInfo(m_xmlPath);
    if (rawInfo.isFile()) {
        return rawInfo.absoluteFilePath();
    }
    return QDir(m_baseDirectory).filePath("dblp.xml");
}

quint32 Searcher::hash4(const QString& text) const
{
    quint64 value = 0;
    const QByteArray bytes = text.toUtf8();
    for (char ch : bytes) {
        value = ((value >> 8) & 0xf) ^ ((value << 4) ^ static_cast<unsigned char>(ch));
    }
    return static_cast<quint32>(value & 0xfff);
}

QVector<QString> Searcher::readLines(const QString& filePath) const
{
    QVector<QString> lines;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return lines;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = safeLineRead(stream);
        if (!line.trimmed().isEmpty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

SearchResult Searcher::parseArticleLine(const QString& line) const
{
    SearchResult result;
    const int splitIndex = line.indexOf(" <-> ");
    if (splitIndex <= 0) {
        return result;
    }

    bool ok = false;
    const qint64 offset = line.mid(splitIndex + 5).trimmed().toLongLong(&ok);
    if (!ok) {
        return result;
    }

    result.title = line.left(splitIndex).trimmed();
    result.offset = offset;
    return result;
}

SearchResult Searcher::parseAuthorLine(const QString& line) const
{
    SearchResult result;
    const int dollarIndex = line.indexOf('$');
    if (dollarIndex <= 0) {
        return result;
    }

    const QString title = line.left(dollarIndex).trimmed();
    const QString tail = line.mid(dollarIndex + 1);
    const int hashIndex = tail.indexOf('#');
    bool ok = false;
    qint64 offset = -1;
    QString author;
    if (hashIndex >= 0) {
        offset = tail.left(hashIndex).trimmed().toLongLong(&ok);
        author = tail.mid(hashIndex + 1).trimmed();
    } else {
        offset = tail.trimmed().toLongLong(&ok);
    }

    if (!ok) {
        return result;
    }

    result.title = title;
    result.authors = author;
    result.offset = offset;
    return result;
}

void Searcher::parsePaperXml(const QString& xml, SearchResult& result) const
{
    const QString parsedTitle = extractSingleTag(xml, "title");
    if (!parsedTitle.isEmpty()) {
        result.title = parsedTitle;
    }

    const QStringList authors = extractMultiTag(xml, "author");
    if (!authors.isEmpty()) {
        result.authors = authors.join(", ");
    }

    QString year = extractSingleTag(xml, "year");
    const QString month = extractSingleTag(xml, "month");
    if (!month.isEmpty() && !year.isEmpty()) {
        result.year = month + " " + year;
    } else {
        result.year = year;
    }

    result.venue = extractSingleTag(xml, "booktitle");
    result.journal = extractSingleTag(xml, "journal");
    if (result.journal.isEmpty()) {
        result.journal = extractSingleTag(xml, "conference");
    }
    if (result.journal.isEmpty()) {
        result.journal = extractSingleTag(xml, "proceedings");
    }

    result.volume = extractSingleTag(xml, "volume");
    result.number = extractSingleTag(xml, "number");
    result.pages = extractSingleTag(xml, "pages");

    const QString ee = extractSingleTag(xml, "ee");
    if (!ee.isEmpty()) {
        result.doi = ee;
    }
}

QString Searcher::extractSingleTag(const QString& xml, const QString& tag) const
{
    QRegularExpression re(QString("<%1[^>]*>(.*?)</%1>").arg(tag),
                          QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = re.match(xml);
    if (!match.hasMatch()) {
        return QString();
    }
    return decodeXmlEntities(match.captured(1));
}

QStringList Searcher::extractMultiTag(const QString& xml, const QString& tag) const
{
    QStringList values;
    QRegularExpression re(QString("<%1[^>]*>(.*?)</%1>").arg(tag),
                          QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(xml);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        values.push_back(decodeXmlEntities(match.captured(1)));
    }
    return values;
}

int Searcher::parseYearValue(const QString& yearText) const
{
    QRegularExpression re("(\\d{4})");
    const QRegularExpressionMatch match = re.match(yearText);
    if (!match.hasMatch()) {
        return 0;
    }
    return match.captured(1).toInt();
}

QVector<int> StringMatcher::kmpSearch(const QString& text, const QString& pattern)
{
    QVector<int> positions;
    if (pattern.isEmpty()) {
        return positions;
    }

    const QVector<int> next = computeNext(pattern);
    int i = 0;
    int j = 0;
    while (i < text.size()) {
        if (j == -1 || text[i] == pattern[j]) {
            ++i;
            ++j;
            if (j == pattern.size()) {
                positions.push_back(i - j);
                j = next[j];
            }
        } else {
            j = next[j];
        }
    }
    return positions;
}

QVector<int> StringMatcher::computeNext(const QString& pattern)
{
    QVector<int> next(pattern.size() + 1);
    next[0] = -1;
    int i = 0;
    int j = -1;
    while (i < pattern.size()) {
        if (j == -1 || pattern[i] == pattern[j]) {
            ++i;
            ++j;
            next[i] = j;
        } else {
            j = next[j];
        }
    }
    return next;
}

} // namespace search_module
