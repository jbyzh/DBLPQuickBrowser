#include "cliquechart.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

CliqueChart::CliqueChart(QWidget *parent, const std::vector<long long>& cliqueCounts, const std::map<int, int>& componentSizeDistribution)
    : QWidget(parent),
      m_cliqueCounts(cliqueCounts),
      m_componentSizeDistribution(componentSizeDistribution),
      m_cliqueChartView(nullptr),
      m_componentChartView(nullptr)
{
    setWindowTitle("聚团分析柱状图");
    setFixedSize(1000, 600);
    setObjectName("WindowBg1");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("#WindowBg1 { border-image: url(:/picture/bg.png); }");

    setupUI();
}

CliqueChart::~CliqueChart()
{
    delete m_cliqueChartView;
    delete m_componentChartView;
}

void CliqueChart::setupUI()
{
    // 创建返回按钮
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

    // 创建标题
    QLabel *titleLabel = new QLabel("聚团分析柱状图", this);
    titleLabel->setGeometry(400, 20, 200, 30);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-family: '楷体', 'KaiTi';"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    color: #333333;"
        "}"
        );
    titleLabel->setAlignment(Qt::AlignCenter);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 60, 20, 20);

    // 创建柱状图
    createCliqueChart();
    createComponentChart();

    // 添加图表到布局
    mainLayout->addWidget(new QLabel("各阶完全子图数量分布", this));
    mainLayout->addWidget(m_cliqueChartView);
    mainLayout->addWidget(new QLabel("连通分量大小分布", this));
    mainLayout->addWidget(m_componentChartView);

    setLayout(mainLayout);
}

void CliqueChart::createCliqueChart()
{
    m_cliqueChartView = new QtCharts::QChartView(this);
    m_cliqueChartView->setFixedSize(960, 200);
    m_cliqueChartView->setStyleSheet("QChartView { background-color: rgba(255, 255, 255, 0.8); border-radius: 6px; }");

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->setTitle("各阶完全子图数量分布");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QtCharts::QBarSet *set = new QtCharts::QBarSet("完全子图数量");

    std::vector<int> orders;
    for (size_t i = 1; i < m_cliqueCounts.size(); i++) {
        if (m_cliqueCounts[i] > 0) {
            *set << m_cliqueCounts[i];
            orders.push_back(i);
        }
    }

    series->append(set);
    chart->addSeries(series);

    // 设置X轴
    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    axisX->setTitleText("阶数");
    axisX->setLabelFormat("%d");

    if (!orders.empty()) {
        axisX->setMin(orders.front() - 0.5);
        axisX->setMax(orders.back() + 0.5);
        axisX->setTickCount(orders.size());
    }

    // 设置Y轴
    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("数量");
    axisY->setLabelFormat("%lld");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_cliqueChartView->setChart(chart);
}

void CliqueChart::createComponentChart()
{
    m_componentChartView = new QtCharts::QChartView(this);
    m_componentChartView->setFixedSize(960, 200);
    m_componentChartView->setStyleSheet("QChartView { background-color: rgba(255, 255, 255, 0.8); border-radius: 6px; }");

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->setTitle("连通分量大小分布");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries();
    QtCharts::QBarSet *set = new QtCharts::QBarSet("连通分量数量");

    std::vector<int> sizes;
    for (const auto& entry : m_componentSizeDistribution) {
        int size = entry.first;
        int count = entry.second;
        *set << count;
        sizes.push_back(size);
    }

    series->append(set);
    chart->addSeries(series);

    // 设置X轴
    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    axisX->setTitleText("连通分量大小");
    axisX->setLabelFormat("%d");

    if (!sizes.empty()) {
        axisX->setMin(sizes.front() - 0.5);
        axisX->setMax(sizes.back() + 0.5);
        axisX->setTickCount(sizes.size());
    }

    // 设置Y轴
    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("数量");
    axisY->setLabelFormat("%d");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    m_componentChartView->setChart(chart);
}