#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>

#include "functionpage.h"
#include "XmlParser.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    XmlParser* m_parser;
    FunctionPage* m_functionPage;

    QLineEdit* m_pathEdit;
    QPushButton* m_parseBtn;
    QTextEdit* m_logText;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
};

#endif // MAINWINDOW_H
