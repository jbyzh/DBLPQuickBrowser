#include "barchart.h"
#include "ui_barchart.h"
#include <cmath>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QBarCategoryAxis>
#include <QMouseEvent>
#include <QDebug>
#include <QFrame>
#include <QLegend>
#include <algorithm>

namespace {

void applyAnalyticsChartStyle(QChart *chart, QChartView *chartView)
{
    if (!chart || !chartView) {
        return;
    }

    chart->setBackgroundBrush(QColor(255, 255, 255, 195));
    chart->setBackgroundPen(QPen(QColor(255, 255, 255, 115), 1));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setPlotAreaBackgroundBrush(QColor(255, 255, 255, 168));
    chart->setPlotAreaBackgroundPen(QPen(QColor(220, 223, 230, 160), 1));
    chart->setTitleBrush(QBrush(QColor("#1F2D3D")));

    if (chart->legend()) {
        chart->legend()->setBackgroundVisible(false);
        chart->legend()->setLabelBrush(QBrush(QColor("#303133")));
    }

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFrameShape(QFrame::NoFrame);
    chartView->setBackgroundBrush(Qt::NoBrush);
    chartView->setStyleSheet("background: transparent;");
}

}

BarChart::BarChart(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BarChart)
{
    ui->setupUi(this);
    
    setFixedSize(1200, 650);
    setWindowTitle(QString::fromUtf8("聚团分析柱状图"));
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("BarChartWindow");
    setStyleSheet(
        "#BarChartWindow{border-image:url(:/picture/bg.png);}"
        "QWidget{background:transparent;}"
        "QGraphicsView#graphicsView{background-color:rgba(255,255,255,0.24); border:1px solid rgba(255,255,255,0.34); border-radius:12px;}"
        "QPushButton#btn_close{background-color:rgba(255,255,255,0.78); color:#303133; border:1px solid rgba(200,200,200,0.45); border-radius:8px; font:600 13px 'Microsoft YaHei'; padding:6px 12px;}"
        "QPushButton#btn_close:hover{background-color:rgba(255,255,255,0.92); border:1px solid #409EFF; color:#409EFF;}"
    );
    
    createBarChart();

    // 去掉默认标题栏，这样窗口可以更自由拖动
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // 但保留窗口关闭等基本功能
    setAttribute(Qt::WA_StyledBackground, true);

    // 连接关闭按钮
    connect(ui->btn_close, &QPushButton::clicked, this, &QWidget::close);
}

BarChart::~BarChart()
{
    delete ui;
}

// 解析科学计数法字符串为double
double BarChart::parseScientificNotation(const QString &str)
{
    if (str.isEmpty()) return 0;
    
    int ePos = str.indexOf('e', Qt::CaseInsensitive);
    if (ePos == -1) {
        return str.toDouble();
    }
    
    QString mantissaStr = str.left(ePos);
    QString exponentStr = str.mid(ePos + 1);
    
    double mantissa = mantissaStr.toDouble();
    int exponent = exponentStr.toInt();
    
    return mantissa * std::pow(10.0, exponent);
}

void BarChart::setFullSubgraphData(const QMap<int, QString> &fullSubgraphData)
{
    // 过滤数据：只显示大于1e5的值
    QMap<int, double> filteredData;
    for (auto it = fullSubgraphData.constBegin(); it != fullSubgraphData.constEnd(); ++it) {
        double value = parseScientificNotation(it.value());
        if (value > 1e5) {  // 下限：10^5
            filteredData[it.key()] = value;
        }
    }
    
    m_fullSubgraphData = filteredData;
    m_useLogScale = true;
    createLogBarChart(filteredData);
}

void BarChart::createLogBarChart(const QMap<int, double> &data)
{
    // 清空graphicsView中的现有内容
    QLayout *oldLayout = ui->graphicsView->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    // 1. 数据
    QBarSet *set = new QBarSet("完全子图数量");
    set->setColor(QColor(54, 162, 235));
    set->setLabelColor(QColor(0, 0, 0));
    
    QStringList categories;
    double maxValue = 1e5;
    int maxOrder = 0;

    if (data.isEmpty()) {
        categories = {"无数据"};
        *set << 0;
        maxValue = 1;
    } else {
        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            set->append(it.value());
            categories << QString::number(it.key());
            if (it.value() > maxValue) {
                maxValue = it.value();
            }
            if (it.key() > maxOrder) {
                maxOrder = it.key();
            }
        }
    }

    // 2. 柱状图系列
    QBarSeries *series = new QBarSeries();
    series->append(set);
    series->setLabelsVisible(false);
    series->setBarWidth(1.0);

    // 3. 图表
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("各阶完全子图数量");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setMargins(QMargins(80, 60, 80, 100));

    // 4. X轴
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0.5, maxOrder + 0.5);
    axisX->setTitleText("阶数");
    axisX->setTitleFont(QFont("Arial", 14, QFont::Bold));
    axisX->setTitleBrush(QColor(0, 0, 0));
    axisX->setLabelsColor(QColor(0, 0, 0));
    axisX->setLabelsFont(QFont("Arial", 10));
    axisX->setTickCount(qMin(maxOrder / 5 + 1, 20));
    axisX->setLabelFormat("%d");
    axisX->setGridLineColor(QColor(200, 200, 200));
    axisX->setGridLineVisible(true);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 5. Y轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(5, 30);
    axisY->setTitleText("log10(数量)");
    axisY->setTitleFont(QFont("Arial", 14, QFont::Bold));
    axisY->setTitleBrush(QColor(0, 0, 0));
    axisY->setLabelsColor(QColor(0, 0, 0));
    axisY->setLabelsFont(QFont("Arial", 10));
    axisY->setTickCount(10);
    axisY->setLabelFormat("%d");
    axisY->setGridLineColor(QColor(200, 200, 200));
    axisY->setGridLineVisible(true);
    chart->addAxis(axisY, Qt::AlignLeft);

    // 将数据转换为对数并附加到Y轴
    QBarSeries *logSeries = new QBarSeries();
    QBarSet *logSet = new QBarSet("完全子图数量(log10)");
    logSet->setColor(QColor(54, 162, 235));

    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        *logSet << log10(it.value());
    }
    logSeries->append(logSet);

    chart->addSeries(logSeries);
    logSeries->attachAxis(axisX);
    logSeries->attachAxis(axisY);

    // 隐藏原始数据系列
    series->hide();

    // 6. 图例
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setFont(QFont("Arial", 12, QFont::Bold));
    chart->legend()->setLabelColor(QColor(0, 0, 0));

    // 7. 显示到 graphicsView
    QChartView *chartView = new QChartView(chart);
    applyAnalyticsChartStyle(chart, chartView);
    
    QVBoxLayout *layout = new QVBoxLayout(ui->graphicsView);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(chartView);
    ui->graphicsView->setLayout(layout);
}

void BarChart::setCliqueData(const QMap<int, qlonglong> &cliqueData)
{
    m_cliqueData = cliqueData;
    createBarChart();
}

void BarChart::createBarChart()
{
    // 清空graphicsView中的现有内容
    QLayout *oldLayout = ui->graphicsView->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    // 1. 数据
    QBarSet *set = new QBarSet("完全子图数量");
    set->setColor(QColor(54, 162, 235));
    set->setLabelColor(QColor(0, 0, 0));
    
    QStringList categories;
    qlonglong maxValue = 0;

    if (m_cliqueData.isEmpty()) {
        categories = {"无数据"};
        *set << 0;
        maxValue = 1;
    } else {
        for (auto it = m_cliqueData.constBegin(); it != m_cliqueData.constEnd(); ++it) {
            set->append(it.value());
            categories << QString::number(it.key());
            if (it.value() > maxValue) {
                maxValue = it.value();
            }
        }
    }

    // 2. 柱状图系列
    QBarSeries *series = new QBarSeries();
    series->append(set);
    series->setLabelsVisible(false);
    series->setBarWidth(0.8);

    // 3. 图表
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("各阶完全子图数量");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setMargins(QMargins(60, 40, 60, 100));

    // 4. X轴
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("阶数");
    axisX->setTitleFont(QFont("Arial", 10, QFont::Bold));
    axisX->setLabelsAngle(45);
    axisX->setLabelsColor(QColor(0, 0, 0));
    axisX->setLabelsFont(QFont("Arial", 8));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 5. Y轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxValue * 1.1);
    axisY->setTitleText("数量");
    axisY->setTitleFont(QFont("Arial", 10, QFont::Bold));
    axisY->setLabelsColor(QColor(0, 0, 0));
    axisY->setLabelsFont(QFont("Arial", 8));
    axisY->setTickCount(6);
    axisY->setLabelFormat("%.0f");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 6. 图例
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);

    // 7. 显示到 graphicsView
    QChartView *chartView = new QChartView(chart);
    applyAnalyticsChartStyle(chart, chartView);
    
    QVBoxLayout *layout = new QVBoxLayout(ui->graphicsView);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(chartView);
    ui->graphicsView->setLayout(layout);
}

void BarChart::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查点击位置是否在窗口内部（特别是不包含关闭按钮区域）
        // 先获取关闭按钮的全局位置
        QRect closeBtnRect = ui->btn_close->geometry();
        QPoint globalCloseBtnPos = ui->btn_close->mapToGlobal(closeBtnRect.topLeft());
        QRect globalCloseBtnRect(globalCloseBtnPos, closeBtnRect.size());
        
        // 如果点击在关闭按钮上，不启动拖动，让按钮自己处理
        const QPoint globalPos = event->globalPosition().toPoint();
        if (!globalCloseBtnRect.contains(globalPos)) {
            m_dragging = true;
            m_dragStartPosition = globalPos - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void BarChart::mouseMoveEvent(QMouseEvent *event)
{
    // 如果正在拖动，并且是左键
    if (m_dragging && event->buttons() & Qt::LeftButton) {
        // 移动窗口到新位置
        move(event->globalPosition().toPoint() - m_dragStartPosition);
        event->accept();
    }
}

void BarChart::mouseReleaseEvent(QMouseEvent *event)
{
    // 结束拖动
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}
