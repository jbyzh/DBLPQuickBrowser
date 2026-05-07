#include "functionpage.h"

#include <QMessageBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

#include "analyticswindow.h"
#include "clique.h"
#include "fuzzysearchdialog.h"
#include "intervalwindow.h"
#include "precise.h"
#include "searchdialog.h"
#include "ui_functionpage.h"

namespace {

QString dataRootFromPath(const QString& dataPath)
{
    const QFileInfo info(dataPath);
    if (info.isFile()) {
        return info.absolutePath();
    }
    if (info.isDir()) {
        return info.absoluteFilePath();
    }
    return info.absolutePath();
}

}

FunctionPage::FunctionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FunctionPage)
{
    ui->setupUi(this);
    ui->pushButton->setText(QString::fromUtf8("基础文献搜索"));
    ui->pushButton_2->setText(QString::fromUtf8("合作关系网络"));
    ui->pushButton_4->setText(QString::fromUtf8("关键词模糊搜索"));
    ui->pushButton_3->setText(QString::fromUtf8("高产作者Top100 / 年度学术热点"));
    ui->pushButton_5->setText(QString::fromUtf8("时间区间分析"));
    ui->pushButton_6->setText(QString::fromUtf8("合作聚团分析"));
    ui->pushButton_7->setText(QString::fromUtf8("智能推荐"));
    ui->pushButton_8->setText(QString::fromUtf8("学术协作图谱"));
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    setWindowTitle(QString::fromUtf8("DBLP XML 功能菜单"));
    setFixedSize(700, 500);
    setObjectName("WindowBg1");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("#WindowBg1 { border-image: url(:/picture/bg.png); }");

    QPushButton *btnBack = new QPushButton(QString::fromUtf8("返回"), this);
    btnBack->setGeometry(20, 20, 80, 35);
    btnBack->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(255, 255, 255, 0.7);"
        "font-family: 'KaiTi';"
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #333333;"
        "border: 1px solid rgba(200, 200, 200, 0.5);"
        "border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(255, 255, 255, 0.9);"
        "border: 1px solid #409eff;"
        "color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(230, 230, 230, 0.8);"
        "}"
    );

    connect(btnBack, &QPushButton::clicked, this, [this]() {
        close();
        emit backToMain();
    });

    m_helpButton = new QPushButton(QString::fromUtf8("功能说明"), this);
    m_helpButton->setGeometry(width() - 108, height() - 44, 82, 28);
    m_helpButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(255, 255, 255, 0.76);"
        "font-family: 'KaiTi';"
        "font-size: 13px;"
        "font-weight: bold;"
        "color: #333333;"
        "border: 1px solid rgba(200, 200, 200, 0.5);"
        "border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(255, 255, 255, 0.92);"
        "border: 1px solid #409eff;"
        "color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(230, 230, 230, 0.8);"
        "}"
    );
    connect(m_helpButton, &QPushButton::clicked, this, &FunctionPage::showFeatureHelp);
}

FunctionPage::~FunctionPage()
{
    delete ui;
}

void FunctionPage::setDataPath(const QString& dataPath)
{
    m_dataPath = dataPath;
}

void FunctionPage::on_pushButton_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    SearchDialog dialog(m_dataPath, this);
    dialog.exec();
}

void FunctionPage::on_pushButton_2_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_preciseWindow) {
        m_preciseWindow->close();
    }
    m_preciseWindow = new Precise(nullptr, dataRootFromPath(m_dataPath), Precise::ModeF2);
    m_preciseWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(m_preciseWindow, &QObject::destroyed, this, [this]() { m_preciseWindow = nullptr; });
    m_preciseWindow->show();
}

void FunctionPage::on_pushButton_4_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    FuzzySearchDialog dialog(m_dataPath, this);
    dialog.exec();
}

void FunctionPage::on_pushButton_3_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_analyticsWindow) {
        m_analyticsWindow->close();
    }
    m_analyticsWindow = new AnalyticsWindow(m_dataPath, this);
    m_analyticsWindow->setInitialTab(0);
    m_analyticsWindow->show();
}

void FunctionPage::on_pushButton_5_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_intervalWindow) {
        m_intervalWindow->close();
    }
    m_intervalWindow = new IntervalWindow(m_dataPath, this);
    m_intervalWindow->show();
}

void FunctionPage::on_pushButton_6_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_cliqueWindow) {
        m_cliqueWindow->close();
    }
    m_cliqueWindow = new Clique(this);
    m_cliqueWindow->show();
}

void FunctionPage::on_pushButton_7_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_preciseWindow) {
        m_preciseWindow->close();
    }
    m_preciseWindow = new Precise(nullptr, dataRootFromPath(m_dataPath), Precise::ModeF9);
    m_preciseWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(m_preciseWindow, &QObject::destroyed, this, [this]() { m_preciseWindow = nullptr; });
    m_preciseWindow->show();
}

void FunctionPage::on_pushButton_8_clicked()
{
    if (m_dataPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先在首页选择并解析 dblp.xml。"));
        return;
    }
    if (m_preciseWindow) {
        m_preciseWindow->close();
    }
    m_preciseWindow = new Precise(nullptr, dataRootFromPath(m_dataPath), Precise::ModeGraph);
    m_preciseWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(m_preciseWindow, &QObject::destroyed, this, [this]() { m_preciseWindow = nullptr; });
    m_preciseWindow->show();
}

void FunctionPage::showFeatureHelp()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("功能说明"));
    dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
    dialog.setFixedSize(650, 520);
    dialog.setObjectName("FeatureHelpDialog");
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet(
        "#FeatureHelpDialog { border-image: url(:/picture/bg.png); }"
        "QWidget#HelpPanel {"
        "    background-color: rgba(255, 255, 255, 0.84);"
        "    border: 1px solid rgba(255, 255, 255, 0.45);"
        "    border-radius: 10px;"
        "}"
        "QLabel { color: #1f2937; }"
        "QTextEdit {"
        "    background-color: rgba(255, 255, 255, 0.92);"
        "    border: 1px solid rgba(170, 185, 205, 0.75);"
        "    border-radius: 8px;"
        "    color: #1f2937;"
        "    padding: 8px;"
        "}"
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.82);"
        "    border: 1px solid rgba(170, 185, 205, 0.8);"
        "    border-radius: 6px;"
        "    padding: 6px 16px;"
        "    font-weight: 600;"
        "    color: #1f2937;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.95);"
        "    border-color: #409eff;"
        "    color: #2563eb;"
        "}"
    );

    auto* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 14, 14, 14);

    auto* panel = new QWidget(&dialog);
    panel->setObjectName("HelpPanel");
    panel->setAttribute(Qt::WA_StyledBackground, true);
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(14, 14, 14, 14);
    panelLayout->setSpacing(10);

    auto* title = new QLabel(QString::fromUtf8("各功能说明"), panel);
    title->setStyleSheet("QLabel { font-size: 22px; font-weight: 700; background: transparent; }");
    panelLayout->addWidget(title);

    auto* textEdit = new QTextEdit(panel);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(
        QString::fromUtf8(
            "1. 基础文献搜索\n"
            "   支持按作者名或完整论文标题检索文献。作者搜索适合查某位学者发过哪些论文；标题搜索适合已知论文名时快速查看详细信息。\n\n"
            "2. 合作关系网络\n"
            "   输入作者名后，展示与该作者有合作关系的其他作者，并按合作次数统计。下方柱状图会直观看到合作最频繁的对象。\n\n"
            "3. 关键词模糊搜索\n"
            "   输入若干关键词后，在标题中做部分匹配搜索。除了结果列表，还会统计命中文献的年份分布，方便看这个主题集中出现在哪些年份。\n\n"
            "4. 高产作者Top100 / 年度学术热点\n"
            "   这个模块分成两个标签页：\n"
            "   - 高产作者Top100：统计发文数量最多的作者，并提供表格和柱状图。\n"
            "   - 年度学术热点：查看某一年标题里最常见的关键词，也能做多年趋势对比。\n\n"
            "5. 时间区间分析\n"
            "   选择起止年份后，查看该时间段的总发文量、峰值年份、年度趋势，以及区间热点关键词。适合看某个时期的发展变化。\n\n"
            "6. 合作聚团分析\n"
            "   基于作者合作图统计不同阶数的完全子图，也就是不同规模的聚团数量。适合从图论角度分析学术合作结构。\n\n"
            "7. 智能推荐\n"
            "   输入一个作者名后，系统会根据合作网络结构推荐可能值得关注的作者。推荐结果会结合合作关系和作者影响力进行排序。\n\n"
            "8. 学术协作图谱\n"
            "   输入目标学者后，构建该作者附近的一跳合作子图，并支持继续扩展节点。适合直观看作者周围的学术合作网络。\n\n"
            "补充说明\n"
            "   如果某些功能第一次打开稍慢，这是因为系统需要先读取已经解析好的索引数据。等加载完成后，再次使用通常会更快。")
    );
    panelLayout->addWidget(textEdit, 1);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    auto* okButton = new QPushButton(QString::fromUtf8("我知道了"), panel);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonRow->addWidget(okButton);
    panelLayout->addLayout(buttonRow);

    root->addWidget(panel);
    dialog.exec();
}
