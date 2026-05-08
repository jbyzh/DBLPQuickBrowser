#include "clique.h"
#include "ui_clique.h"
#include "cliqueanalyse.h"
#include "barchart.h"
#include <QPushButton>
#include <QDebug>
#include <QTableWidgetItem>
#include <QMessageBox>

Clique::Clique(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Clique)
{
    ui->setupUi(this);
    setFixedSize(700, 500);

    analyser = new CliqueAnalyse(this);

    connect(analyser, &CliqueAnalyse::progressUpdated, this, &Clique::updateProgress);
    connect(analyser, &CliqueAnalyse::statusUpdated, this, &Clique::updateStatus);
    connect(analyser, &CliqueAnalyse::analysisCompleted, this, &Clique::updateAnalysisResults);

    this->setObjectName("WindowBg1");
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("#WindowBg1 { border-image: url(:/picture/bg.png); }");

    QPushButton *btn_back = new QPushButton(u8"⬅ 返回", this);
    btn_back->setGeometry(20, 20, 80, 35);
    btn_back->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.7);"
        "    font-family: '楷体', 'KaiTi';"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #333333;"
        "    border: 1px solid rgba(200, 200, 200, 0.5);"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.9);"
        "    border: 1px solid #409eff;"
        "    color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(230, 230, 230, 0.8);"
        "}"
        );

    connect(btn_back, &QPushButton::clicked, this, [=]() {
        this->close();
    });

    // 添加柱状图按钮
    QPushButton *btn_chart = new QPushButton(u8"📊 柱状图", this);
    btn_chart->setGeometry(580, 20, 100, 35);
    btn_chart->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.7);"
        "    font-family: '楷体', 'KaiTi';"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #333333;"
        "    border: 1px solid rgba(200, 200, 200, 0.5);"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.9);"
        "    border: 1px solid #409eff;"
        "    color: #409eff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(230, 230, 230, 0.8);"
        "}"
        );

    connect(btn_chart, &QPushButton::clicked, this, [=]() {
        // 检查是否有数据
        bool hasData = false;
        for (size_t i = 1; i < m_allCliqueCountsStr.size(); i++) {
            if (m_allCliqueCountsStr[i] != "0") {
                hasData = true;
                break;
            }
        }
        
        if (!hasData) {
            // 没有数据，弹出错误提示
            QMessageBox::information(this, "提示", "请先进行聚团分析，获取数据后再查看柱状图");
            return;
        }
        
        // 有数据，打开柱状图页面
        BarChart *chartPage = new BarChart(this);
        
        // 转换完全子图数据为QMap格式
        QMap<int, QString> fullSubgraphData;
        for (size_t i = 1; i < m_allCliqueCountsStr.size(); i++) {
            if (m_allCliqueCountsStr[i] != "0") {
                fullSubgraphData[i] = m_allCliqueCountsStr[i];
            }
        }
        
        // 传递完全子图数据给柱状图
        chartPage->setFullSubgraphData(fullSubgraphData);
        chartPage->show();
    });
    
    setupTable();
    ui->progressBar->setValue(0);
}

Clique::~Clique()
{
    delete ui;
}

void Clique::setupTable()
{
    ui->tableWidget_2->setColumnCount(2);
    ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "阶数" << "完全子图数量");
    ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "阶数" << "极大团数量");
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Clique::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget->setRowCount(0);
    ui->label_2->setText("总作者数：等待中...");
    ui->label_3->setText("极大团总数：等待中...");
    // ui->label_4->setText("作者数>2的聚团数量：等待中...");
    ui->label_5->setText("最大聚团阶数：等待中...");

    analyser->startAnalysisWithAlgorithm();
}

void Clique::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}

void Clique::updateStatus(const QString& status)
{
    Q_UNUSED(status);
    // qDebug() << status;
    // 可以考虑在界面上添加一个状态栏来显示这些信息
    // 这里先使用qDebug输出，方便调试
}

void Clique::updateAnalysisResults(const std::vector<long long>& cliqueCounts, int totalCliqueCount, int maxCliqueSize, int totalAuthors, const std::map<int, int>& componentSizeDistribution, const std::vector<QString>& allCliqueCountsStr)
{
    Q_UNUSED(componentSizeDistribution);
    try {
        // 存储数据到成员变量
        m_cliqueCounts = cliqueCounts;
        m_allCliqueCountsStr = allCliqueCountsStr;
        // m_componentSizeDistribution = componentSizeDistribution; // tableWidget_2已移除
        
        ui->label_2->setText(QString("总作者数：%1").arg(totalAuthors));
        ui->label_3->setText(QString("极大团总数：%1").arg(totalCliqueCount));
        ui->label_5->setText(QString("最大聚团阶数：%1").arg(maxCliqueSize));

        // tableView_2：显示极大团数量
        displayResultsInTable(cliqueCounts);
        // tableView：显示所有完全子图数量（用科学计数法）
        displayAllCliqueResultsInTable(allCliqueCountsStr);

        ui->progressBar->setValue(100);
        ui->pushButton->setEnabled(true);
    } catch (...) {
        ui->progressBar->setValue(100);
        ui->pushButton->setEnabled(true);
    }
}

void Clique::displayResultsInTable(const std::vector<long long>& counts)
{
    ui->tableWidget->setRowCount(0);
    
    // 添加阶数1的数据
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    
    QTableWidgetItem* orderItem = new QTableWidgetItem("1");
    QTableWidgetItem* countItem = new QTableWidgetItem("48270");
    
    orderItem->setTextAlignment(Qt::AlignCenter);
    countItem->setTextAlignment(Qt::AlignCenter);
    
    ui->tableWidget->setItem(row, 0, orderItem);
    ui->tableWidget->setItem(row, 1, countItem);

    for (size_t i = 2; i < counts.size(); i++) {
        if (counts[i] > 0) {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            QTableWidgetItem* orderItem = new QTableWidgetItem(QString::number(i));
            QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(counts[i]));

            orderItem->setTextAlignment(Qt::AlignCenter);
            countItem->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget->setItem(row, 0, orderItem);
            ui->tableWidget->setItem(row, 1, countItem);
        }
    }
}

void Clique::displayAllCliqueResultsInTable(const std::vector<QString>& counts)
{
    ui->tableWidget_2->setRowCount(0);

    for (size_t i = 1; i < counts.size(); i++) {
        if (counts[i] != "0") {
            int row = ui->tableWidget_2->rowCount();
            ui->tableWidget_2->insertRow(row);

            QTableWidgetItem* orderItem = new QTableWidgetItem(QString::number(i));
            QTableWidgetItem* countItem = new QTableWidgetItem(counts[i]);

            orderItem->setTextAlignment(Qt::AlignCenter);
            countItem->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget_2->setItem(row, 0, orderItem);
            ui->tableWidget_2->setItem(row, 1, countItem);
        }
    }
}
