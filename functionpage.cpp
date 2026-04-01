#include "FunctionPage.h"
#include <QDebug>
#include "ui_functionpage.h" // 必须包含这个自动生成的头文件

// 1. 修改构造函数初始化列表
FunctionPage::FunctionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FunctionPage) // 初始化 ui 指针
{
    // 2. 核心：加载 UI 界面
    ui->setupUi(this);

    // 设置窗口基本属性
    setWindowTitle(u8"DBLP XML 功能菜单");
    setFixedSize(1050, 600);

    // 3. 重新连接信号槽 (注意：现在要通过 ui-> 来找控件)
    // 假设你在 .ui 设计器里给按钮起的 objectName 分别是 authorBtn, yearBtn 等

    /*connect(ui->authorBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << u8"作者查询按钮被点击";
    });

    connect(ui->yearBtn, &QPushButton::clicked, this, [=]() { qDebug() << u8"年份统计按钮被点击"; });

    connect(ui->articleBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << u8"文章检索按钮被点击";
    });

    // 返回主界面
    connect(ui->backBtn, &QPushButton::clicked, this, &FunctionPage::backToMain);*/
}

// 4. 必须实现析构函数，释放内存
FunctionPage::~FunctionPage()
{
    delete ui;
}
