#include "FunctionPage.h"
#include <QDebug>
#include "ui_functionpage.h"
#include "mainwindow.h"
#include "precise.h"
// 1. 修改构造函数初始化列表
FunctionPage::FunctionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FunctionPage)
{

    ui->setupUi(this);

    setWindowTitle(u8"DBLP XML 功能菜单");
    setFixedSize(700, 500);

    //通过样式表设置背景
    this->setObjectName("WindowBg1");

    // 强制激活 QWidget 的 QSS 背景绘制能力！
    this->setAttribute(Qt::WA_StyledBackground, true);

    // 使用 border-image 可以让图片自动拉伸、缩放，完美铺满整个窗口
    this->setStyleSheet("#WindowBg1 { border-image: url(:/picture/bg.png); }");

    QPushButton *btn_back = new QPushButton(u8"⬅ 返回", this);

    // 2. 设置绝对位置和较小的尺寸
    btn_back->setGeometry(20, 20, 80, 35);

    // 3. 设置样式
    btn_back->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.7);" /* 70% 不透明度 */
        "    font-family: '楷体', 'KaiTi';"
        "    font-size: 16px;"                            /* 字体比主按钮稍微小一点 */
        "    font-weight: bold;"
        "    color: #333333;"
        "    border: 1px solid rgba(200, 200, 200, 0.5);"
        "    border-radius: 6px;"                         /* 圆角弧度也调小一点点 */
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.9);" /* 鼠标悬停时透明度降低，感觉亮起来 */
        "    border: 1px solid #409eff;"
        "    color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(230, 230, 230, 0.8);" /* 按下时稍微变暗 */
        "}"
        );

    connect(btn_back, &QPushButton::clicked, this, [=]() {
        this->close();
        MainWindow *w=new MainWindow();
        w->show();

    });
}


FunctionPage::~FunctionPage()
{
    delete ui;
}

void FunctionPage::on_pushButton_clicked()
{
    this->close();
    Precise *p=new Precise();
    p->show();
}

