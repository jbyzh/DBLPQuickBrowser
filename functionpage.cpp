// FunctionPage.cpp
#include "FunctionPage.h"

FunctionPage::FunctionPage(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(u8"DBLP XML 功能菜单");
    setFixedSize(600, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(50, 50, 50, 50);

    titleLabel = new QLabel(u8"解析完成！请选择功能：", this);
    titleLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#333;");
    titleLabel->setAlignment(Qt::AlignCenter);

    authorBtn = new QPushButton(u8"📝 作者查询", this);
    yearBtn = new QPushButton(u8"📅 年份统计", this);
    articleBtn = new QPushButton(u8"🔍 文章检索", this);
    backBtn = new QPushButton(u8"返回主界面", this);

    auto btnStyle = "QPushButton {font-size:14px; padding:12px; background-color:#409EFF; color:white; border:none; border-radius:4px;}"
                    "QPushButton:hover {background-color:#337ecc;}";
    authorBtn->setStyleSheet(btnStyle);
    yearBtn->setStyleSheet(btnStyle);
    articleBtn->setStyleSheet(btnStyle);
    backBtn->setStyleSheet("QPushButton {font-size:12px; padding:8px; background-color:#6c757d; color:white; border:none; border-radius:4px;}");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(authorBtn);
    mainLayout->addWidget(yearBtn);
    mainLayout->addWidget(articleBtn);
    mainLayout->addStretch();
    mainLayout->addWidget(backBtn);

    connect(authorBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << u8"作者查询按钮被点击";
    });

    connect(yearBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << u8"年份统计按钮被点击";
    });

    connect(articleBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << u8"文章检索按钮被点击";
    });

    connect(backBtn, &QPushButton::clicked, this, &FunctionPage::backToMain);
}
