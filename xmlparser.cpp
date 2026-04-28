#include "xmlparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <iostream>

using namespace data_initial;

XmlParser::XmlParser(QObject *parent)
    : QObject(parent)
    , m_maxThread(4)
    , m_totalThread(8)
    , m_fileCheck(true)
{
    SetConsoleOutputCP(CP_ACP);
    SetConsoleCP(CP_ACP);
    std::ios::sync_with_stdio(true);
    std::cout.imbue(std::locale(""));
}

void XmlParser::setParams(DWORD maxThread, DWORD totalThread, bool fileCheck, const QString &fileUrl)
{
    m_maxThread = maxThread;
    m_totalThread = totalThread;
    m_fileCheck = fileCheck;
    m_fileUrl = fileUrl;
}

bool XmlParser::isFileParsed() const
{
    QFile finishFile(QDir(normalizedBaseDir()).filePath("database/finish.db"));
    return finishFile.exists();
}

void XmlParser::startParse()
{
    emit parseStarted();
    emit parseMessage(QString::fromUtf8("开始解析 dblp.xml..."));
    emit parseProgress(QString::fromUtf8("初始化解析线程..."));

    const QString baseDir = normalizedBaseDir();
    QByteArray baseDirBytes = QDir::toNativeSeparators(baseDir + "/").toLocal8Bit();
    char* filePath = baseDirBytes.data();
    bool result = false;

    try {
        result = data_initial::initial_readers(m_maxThread, m_totalThread, m_fileCheck, filePath);
    } catch (...) {
        emit parseMessage(QString::fromUtf8("[异常] 解析过程中出错。"));
    }

    emit parseFinished(result);

    if (result) {
        emit parseMessage(QString::fromUtf8("解析成功，结果已保存到：") + QDir(baseDir).filePath("database"));
    } else {
        emit parseMessage(QString::fromUtf8("解析失败。"));
    }
}

QString XmlParser::normalizedBaseDir() const
{
    QFileInfo info(m_fileUrl);
    if (info.isFile()) {
        return info.absolutePath();
    }
    if (info.isDir()) {
        return info.absoluteFilePath();
    }
    return QDir::cleanPath(m_fileUrl);
}
