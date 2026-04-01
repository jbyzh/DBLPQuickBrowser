#include "precise.h"
#include "ui_precise.h"

Precise::Precise(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Precise)
{
    ui->setupUi(this);
    setWindowTitle(u8"精准文献搜索");
    setFixedSize(700, 500);
}

Precise::~Precise()
{
    delete ui;
}
