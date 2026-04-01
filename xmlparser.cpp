#include "XmlParser.h"
#include <QDir>
#include <QDebug>
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
    QFile finishFile(m_fileUrl + "/database/finish.db");
    return finishFile.exists();
}

void XmlParser::startParse()
{
    emit parseStarted();
    emit parseMessage(u8"开始解析dblp.xml...");
    emit parseProgress(u8"初始化解析线程...");

    char* filePath = m_fileUrl.toLocal8Bit().data();
    bool result = false;
    try {
        result = data_initial::initial_readers(
            m_maxThread,
            m_totalThread,
            m_fileCheck,
            filePath
            );
    } catch (...) {
        emit parseMessage(u8"[异常] 解析过程出错！");
    }

    emit parseFinished(result);

    if (result) {
        QString dbPath = m_fileUrl + "/database/";
        emit parseMessage(u8"解析成功！结果已保存至：" + dbPath);
    } else {
        emit parseMessage(u8"解析失败！");
    }
}
