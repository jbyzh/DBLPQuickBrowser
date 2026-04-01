#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include "XmlParser.h"
#include "FunctionPage.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // ########## 关键修改：删除以下冗余的槽函数声明 ##########
    // private slots:
    //     void onParseBtnClicked();
    //     void onParseStarted();
    //     void onParseFinished(bool success);
    //     void onParseMessage(const QString& msg);

private:
    XmlParser* m_parser;
    FunctionPage* m_functionPage; // 功能页面

    // UI控件
    QLineEdit* m_pathEdit;
    QPushButton* m_parseBtn;
    QPushButton* m_funcBtn;
    QTextEdit* m_logText;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
};

#endif // MAINWINDOW_H
