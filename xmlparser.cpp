#include "xmlparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QSettings>
#include <QTimer>
#include <iostream>

#include "GraphManager.h"

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

XmlParser::~XmlParser()
{
    stopProgressTimer();
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
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
    if (m_isParsing) {
        emit parseMessage(QString::fromUtf8("解析任务正在进行中，请稍候..."));
        return;
    }

    m_isParsing = true;
    emit parseStarted();
    emit parseMessage(QString::fromUtf8("开始解析 dblp.xml..."));
    emit parseProgress(QString::fromUtf8("初始化解析线程..."));
    m_lastProgressValue = -1;
    GraphManager::instance().clear();

    const QString baseDir = normalizedBaseDir();
    if (!m_progressTimer) {
        m_progressTimer = new QTimer(this);
        m_progressTimer->setInterval(1000);
        connect(m_progressTimer, &QTimer::timeout, this, [this]() {
            const long long currentValue = data_initial::total_num.load();
            if (currentValue == m_lastProgressValue) {
                return;
            }
            m_lastProgressValue = currentValue;
            emit parseProgress(QString::fromUtf8("已解析记录数：%1").arg(currentValue));
        });
    }
    m_progressTimer->start();

    const DWORD maxThread = m_maxThread;
    const DWORD totalThread = m_totalThread;
    const bool fileCheck = m_fileCheck;
    const QByteArray baseDirBytes = QDir::toNativeSeparators(baseDir + "/").toLocal8Bit();

    m_workerThread = QThread::create([this, maxThread, totalThread, fileCheck, baseDir, baseDirBytes]() {
        bool result = false;
        try {
            result = data_initial::initial_readers(maxThread, totalThread, fileCheck, const_cast<char*>(baseDirBytes.constData()));
        } catch (...) {
            QMetaObject::invokeMethod(this, [this]() {
                emit parseMessage(QString::fromUtf8("[异常] 解析过程中出错。"));
            }, Qt::QueuedConnection);
        }

        QMetaObject::invokeMethod(this, [this, result, baseDir]() {
            stopProgressTimer();
            m_isParsing = false;
            emit parseFinished(result);

            if (result) {
                const QString graphPath = QDir(baseDir).filePath("database/graph");
                GraphManager::instance().saveToFile(graphPath.toLocal8Bit().toStdString());
                QSettings settings("DBLPQuickBrowser", "Settings");
                settings.setValue("databasePath", baseDir);
                emit parseMessage(QString::fromUtf8("解析成功，结果已保存到：") + QDir(baseDir).filePath("database"));
            } else {
                emit parseMessage(QString::fromUtf8("解析失败。"));
            }

            if (m_workerThread) {
                m_workerThread->deleteLater();
                m_workerThread = nullptr;
            }
        }, Qt::QueuedConnection);
    });
    m_workerThread->start();
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

void XmlParser::stopProgressTimer()
{
    if (m_progressTimer && m_progressTimer->isActive()) {
        m_progressTimer->stop();
    }
}
