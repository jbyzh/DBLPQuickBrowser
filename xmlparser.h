#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <windows.h>

#include "Data_initial.h"

class XmlParser : public QObject
{
    Q_OBJECT

public:
    explicit XmlParser(QObject *parent = nullptr);
    ~XmlParser();

    void setParams(DWORD maxThread = 4, DWORD totalThread = 8, bool fileCheck = true, const QString& fileUrl = QString());
    bool isFileParsed() const;
    void startParse();

signals:
    void parseStarted();
    void parseProgress(const QString& msg);
    void parseFinished(bool success);
    void parseMessage(const QString& msg);

private:
    QString normalizedBaseDir() const;
    void stopProgressTimer();

    DWORD m_maxThread;
    DWORD m_totalThread;
    bool m_fileCheck;
    QString m_fileUrl;
    bool m_isParsing = false;
    QThread* m_workerThread = nullptr;
    QTimer* m_progressTimer = nullptr;
    long long m_lastProgressValue = -1;
};

#endif // XMLPARSER_H
