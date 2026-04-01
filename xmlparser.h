#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <windows.h>
// 引入你的解析器头文件
#include "Data_initial.h"

class XmlParser : public QObject
{
    Q_OBJECT
public:
    explicit XmlParser(QObject *parent = nullptr);

    // 设置解析参数
    void setParams(DWORD maxThread = 4, DWORD totalThread = 8, bool fileCheck = true, const QString& fileUrl = "");
    // 检查文件是否已解析（通过finish.db标记判断）
    bool isFileParsed() const;
    // 执行解析（调用你的initial_readers）
    void startParse();
    // 打开指定目录
    //void openDirectory(const QString& path);

signals:
    // 解析状态信号（供UI更新）
    void parseStarted();                  // 解析开始
    void parseProgress(const QString& msg); // 进度消息（如已读取条数）
    void parseFinished(bool success);     // 解析完成
    void parseMessage(const QString& msg); // 通用日志消息

private:
    DWORD m_maxThread;    // 最大线程数
    DWORD m_totalThread;  // 总线程数
    bool m_fileCheck;     // 文件检查标记
    QString m_fileUrl;    // XML文件目录
};

#endif // XMLPARSER_H
