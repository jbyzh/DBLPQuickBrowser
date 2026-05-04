#include "functionpage.h"

#include <QMessageBox>
#include <QPushButton>

#include "analyticswindow.h"
#include "clique.h"
#include "intervalwindow.h"
#include "precise.h"
#include "ui_functionpage.h"

FunctionPage::FunctionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FunctionPage)
{
    ui->setupUi(this);
    ui->pushButton_3->setText(QString::fromUtf8("高产作者Top100 / 年度学术热点"));
    ui->pushButton_5->setText(QString::fromUtf8("时间区间分析"));
    ui->pushButton_6->setText(QString::fromUtf8("合作聚团分析"));
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
    close();
    Precise *preciseWindow = new Precise();
    preciseWindow->show();
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
