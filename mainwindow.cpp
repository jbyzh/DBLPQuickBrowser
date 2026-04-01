#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_parser(new XmlParser(this))
{
    this->setWindowIcon(QIcon(":/picture/book.jpeg"));
    setWindowTitle(u8"DBLP XML解析");
    setFixedSize(700, 500);

    //通过样式表设置背景
    this->setObjectName("WindowBg");
    // 使用 border-image 可以让图片自动拉伸、缩放，完美铺满整个窗口
    this->setStyleSheet("#WindowBg { border-image: url(:/picture/bg.png); }");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel(u8"XML目录：", this);
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setText("E:\\DBLP_Quick_Browser\\dblp.xml\\");
    QPushButton* selectBtn = new QPushButton(u8"选择目录", this);
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(selectBtn);

    QHBoxLayout* threadLayout = new QHBoxLayout();
    QLabel* maxThreadLabel = new QLabel(u8"最大线程数：", this);
    QLineEdit* maxThreadEdit = new QLineEdit("4", this);
    maxThreadEdit->setFixedWidth(50);
    QLabel* totalThreadLabel = new QLabel(u8"总线程数：", this);
    QLineEdit* totalThreadEdit = new QLineEdit("8", this);
    totalThreadEdit->setFixedWidth(50);
    threadLayout->addWidget(maxThreadLabel);
    threadLayout->addWidget(maxThreadEdit);
    threadLayout->addWidget(totalThreadLabel);
    threadLayout->addWidget(totalThreadEdit);
    threadLayout->addStretch();

    // ================= 新增/修改：按钮布局 =================
    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_parseBtn = new QPushButton(u8"开始解析", this);
    m_parseBtn->setStyleSheet("font-size:14px; padding:10px; background-color:#409EFF; color:white; border:none; border-radius:4px;");
    m_parseBtn->setFixedHeight(40);

    // 新增：“进入程序”按钮（换了个绿色主题区分一下）
    QPushButton* m_enterBtn = new QPushButton(u8"进入程序", this);
    m_enterBtn->setStyleSheet("font-size:14px; padding:10px; background-color:#67C23A; color:white; border:none; border-radius:4px;");
    m_enterBtn->setFixedHeight(40);

    btnLayout->addWidget(m_parseBtn);
    btnLayout->addWidget(m_enterBtn); // 把两个按钮放在同一行
    // =======================================================

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->hide();

    m_statusLabel = new QLabel(u8"状态：未开始", this);
    m_statusLabel->setStyleSheet("font-size:12px; color:#666;");

    QLabel* logLabel = new QLabel(u8"解析日志：", this);
    m_logText = new QTextEdit(this);
    m_logText->setReadOnly(true);
    m_logText->setStyleSheet("font-family:Consolas; font-size:11px;");
    m_logText->setPlaceholderText(u8"解析日志将显示在这里...");

    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(threadLayout);
    mainLayout->addLayout(btnLayout); // 添加按钮布局
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(logLabel);
    mainLayout->addWidget(m_logText);
    setCentralWidget(centralWidget);

    m_functionPage = new FunctionPage();
    connect(m_functionPage, &FunctionPage::backToMain, this, [=]() {
        m_functionPage->hide();
        this->show();
    });

    // ================= 新增：“进入程序”按钮逻辑 =================
    connect(m_enterBtn, &QPushButton::clicked, this, [=]() {
        QString filePath = m_pathEdit->text().trimmed();
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, u8"警告", u8"请确认XML文件路径！");
            return;
        }

        // 1. 必须先将当前路径和参数传给 parser！
        // 否则 parser 刚启动是空白状态，根本不知道去哪里检查是否已经解析过
        bool maxOk, totalOk;
        DWORD maxThread = maxThreadEdit->text().toUInt(&maxOk);
        DWORD totalThread = totalThreadEdit->text().toUInt(&totalOk);
        m_parser->setParams(maxThread, totalThread, true, filePath);

        // 2. 告诉了 parser 路径之后，再去判断它底层的文件/数据库是否存在
        if (m_parser->isFileParsed()) {
            this->hide();
            m_functionPage->show();
        } else {
            // 没解析过则弹窗提示
            QMessageBox::warning(this, u8"提示", u8"首次使用请先进行解析！");
        }
    });
    // ============================================================

    connect(m_parseBtn, &QPushButton::clicked, this, [=]() {
        QString filePath = m_pathEdit->text().trimmed();
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            QMessageBox::warning(this, u8"警告", u8"请选择有效的dblp.xml文件路径！");
            return;
        }

        bool maxOk, totalOk;
        DWORD maxThread = maxThreadEdit->text().toUInt(&maxOk);
        DWORD totalThread = totalThreadEdit->text().toUInt(&totalOk);
        if (!maxOk || !totalOk || maxThread > 16 || totalThread < maxThread) {
            QMessageBox::warning(this, u8"参数错误", u8"线程数设置错误！\n最大线程数≤16，总线程数≥最大线程数");
            return;
        }

        m_parser->setParams(maxThread, totalThread, true, filePath);

        if (m_parser->isFileParsed()) {
            QMessageBox::information(this, u8"提示", u8"文件已解析，准备跳转！");
            m_logText->append(u8"[提示] 文件已解析，即将进入功能菜单...");
            m_statusLabel->setText(u8"状态：文件已解析");

            this->hide();
            m_functionPage->show();
            return;
        }

        m_parser->startParse();
    });

    connect(m_parser, &XmlParser::parseMessage, this, [=](const QString& msg) {
        m_logText->append(u8"[信息] " + msg);
    });

    connect(m_parser, &XmlParser::parseStarted, this, [=]() {
        m_parseBtn->setEnabled(false);
        m_enterBtn->setEnabled(false); // 解析时为了安全，禁用进入按钮
        m_progressBar->show();
        m_statusLabel->setText(u8"状态：解析中...");
        m_logText->append(u8"[开始] 解析线程已启动，正在读取XML文件...");
    });

    // ================= 修改：合并后的 parseFinished 逻辑 =================
    connect(m_parser, &XmlParser::parseFinished, this, [=](bool success) {
        m_parseBtn->setEnabled(true);
        m_enterBtn->setEnabled(true); // 解析完成恢复进入按钮
        m_progressBar->hide();

        if (success) {
            m_statusLabel->setText(u8"状态：解析成功");
            m_logText->append(u8"[完成] 所有数据已写入database目录！");
            QMessageBox::information(this, u8"提示", u8"解析完成，准备跳转！");
            this->hide();
            m_functionPage->show();
        } else {
            m_statusLabel->setText(u8"状态：解析失败");
            m_logText->append(u8"[错误] 解析过程中出现异常！");
            QMessageBox::critical(this, u8"错误", u8"XML解析失败！请检查文件路径或线程参数。");
        }
    });
    // =====================================================================
}

MainWindow::~MainWindow()
{
}
