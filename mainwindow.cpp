#include "mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_parser(new XmlParser(this))
{
    setWindowTitle(QString::fromUtf8("DBLP XML 解析"));
    setFixedSize(700, 500);
    setObjectName("WindowBg");
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    setStyleSheet("#WindowBg { border-image: url(:/picture/bg.png); }");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel(QString::fromUtf8("dblp.xml 路径"), this);
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setText("E:\\DBLP_Quick_Browser\\dblp.xml\\");
    //m_pathEdit->setText(QDir::current().filePath("dblp.xml"));
    QPushButton* selectBtn = new QPushButton(QString::fromUtf8("选择文件"), this);
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(selectBtn);

    QHBoxLayout* threadLayout = new QHBoxLayout();
    QLabel* maxThreadLabel = new QLabel(QString::fromUtf8("最大线程数"), this);
    QLineEdit* maxThreadEdit = new QLineEdit("4", this);
    maxThreadEdit->setFixedWidth(60);
    QLabel* totalThreadLabel = new QLabel(QString::fromUtf8("总线程数"), this);
    QLineEdit* totalThreadEdit = new QLineEdit("8", this);
    totalThreadEdit->setFixedWidth(60);
    threadLayout->addWidget(maxThreadLabel);
    threadLayout->addWidget(maxThreadEdit);
    threadLayout->addWidget(totalThreadLabel);
    threadLayout->addWidget(totalThreadEdit);
    threadLayout->addStretch();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_parseBtn = new QPushButton(QString::fromUtf8("开始解析"), this);
    m_parseBtn->setStyleSheet("font-size:14px; padding:10px; background-color:#409EFF; color:white; border:none; border-radius:4px;");
    m_parseBtn->setFixedHeight(40);

    QPushButton* enterBtn = new QPushButton(QString::fromUtf8("进入系统"), this);
    enterBtn->setStyleSheet("font-size:14px; padding:10px; background-color:#67C23A; color:white; border:none; border-radius:4px;");
    enterBtn->setFixedHeight(40);

    btnLayout->addWidget(m_parseBtn);
    btnLayout->addWidget(enterBtn);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->hide();

    m_statusLabel = new QLabel(QString::fromUtf8("状态：未开始"), this);
    m_statusLabel->setStyleSheet("font-size:12px; color:#666;");

    QLabel* logLabel = new QLabel(QString::fromUtf8("解析日志"), this);
    m_logText = new QTextEdit(this);
    m_logText->setReadOnly(true);
    m_logText->setStyleSheet("font-family:Consolas; font-size:11px;");
    m_logText->setPlaceholderText(QString::fromUtf8("解析日志将显示在这里..."));

    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(threadLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(logLabel);
    mainLayout->addWidget(m_logText);
    setCentralWidget(centralWidget);

    m_functionPage = new FunctionPage();
    connect(m_functionPage, &FunctionPage::backToMain, this, [this]() {
        m_functionPage->hide();
        show();
    });

    connect(selectBtn, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            QString::fromUtf8("选择 dblp.xml 文件"),
            m_pathEdit->text(),
            QString::fromUtf8("XML 文件 (*.xml)")
        );
        if (!filePath.isEmpty()) {
            m_pathEdit->setText(filePath);
        }
    });

    connect(enterBtn, &QPushButton::clicked, this, [=]() {
        const QString filePath = m_pathEdit->text().trimmed();
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请先选择 dblp.xml 文件。"));
            return;
        }

        bool maxOk = false;
        bool totalOk = false;
        DWORD maxThread = maxThreadEdit->text().toUInt(&maxOk);
        DWORD totalThread = totalThreadEdit->text().toUInt(&totalOk);
        m_parser->setParams(maxThread, totalThread, true, filePath);
        m_functionPage->setDataPath(filePath);

        if (m_parser->isFileParsed()) {
            hide();
            m_functionPage->show();
        } else {
            QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先完成 XML 解析。"));
        }
    });

    connect(m_parseBtn, &QPushButton::clicked, this, [=]() {
        const QString filePath = m_pathEdit->text().trimmed();
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("请选择有效的 dblp.xml 文件路径。"));
            return;
        }

        bool maxOk = false;
        bool totalOk = false;
        DWORD maxThread = maxThreadEdit->text().toUInt(&maxOk);
        DWORD totalThread = totalThreadEdit->text().toUInt(&totalOk);
        if (!maxOk || !totalOk || maxThread > 16 || totalThread < maxThread) {
            QMessageBox::warning(this, QString::fromUtf8("参数错误"), QString::fromUtf8("线程参数非法：最大线程数不能超过 16，且总线程数不能小于最大线程数。"));
            return;
        }

        m_parser->setParams(maxThread, totalThread, true, filePath);
        m_functionPage->setDataPath(filePath);

        if (m_parser->isFileParsed()) {
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("该文件已解析，直接进入系统。"));
            m_logText->append(QString::fromUtf8("[提示] 已检测到 database/finish.db，直接进入功能菜单。"));
            m_statusLabel->setText(QString::fromUtf8("状态：已解析"));
            hide();
            m_functionPage->show();
            return;
        }

        m_parser->startParse();
    });

    connect(m_parser, &XmlParser::parseMessage, this, [this](const QString& msg) {
        m_logText->append(QString::fromUtf8("[信息] ") + msg);
    });

    connect(m_parser, &XmlParser::parseStarted, this, [this, enterBtn]() {
        m_parseBtn->setEnabled(false);
        enterBtn->setEnabled(false);
        m_progressBar->show();
        m_statusLabel->setText(QString::fromUtf8("状态：解析中..."));
        m_logText->append(QString::fromUtf8("[开始] 解析线程已启动，正在读取 XML 文件..."));
    });

    connect(m_parser, &XmlParser::parseFinished, this, [this, enterBtn](bool success) {
        m_parseBtn->setEnabled(true);
        enterBtn->setEnabled(true);
        m_progressBar->hide();

        if (success) {
            m_statusLabel->setText(QString::fromUtf8("状态：解析成功"));
            m_logText->append(QString::fromUtf8("[完成] 数据已经写入 database 目录。"));
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("解析完成，准备进入功能菜单。"));
            hide();
            m_functionPage->show();
        } else {
            m_statusLabel->setText(QString::fromUtf8("状态：解析失败"));
            m_logText->append(QString::fromUtf8("[错误] 解析过程中出现异常。"));
            QMessageBox::critical(this, QString::fromUtf8("错误"), QString::fromUtf8("XML 解析失败，请检查文件路径或线程参数。"));
        }
    });
}

MainWindow::~MainWindow() = default;
