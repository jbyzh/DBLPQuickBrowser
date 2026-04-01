#include "precise.h"
#include "ui_precise.h"

Precise::Precise(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Precise)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/picture/book.jpeg"));
    setWindowTitle(u8"精准文献搜索");
    setFixedSize(700, 500);
    //通过样式表设置背景
    this->setObjectName("WindowBg1");

    // 强制激活 QWidget 的 QSS 背景绘制能力！
    this->setAttribute(Qt::WA_StyledBackground, true);

    // 使用 border-image 可以让图片自动拉伸、缩放，完美铺满整个窗口
    this->setStyleSheet("#WindowBg1 { border-image: url(:/picture/bg.png); }");
}

Precise::~Precise()
{
    delete ui;
}
