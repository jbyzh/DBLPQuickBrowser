#include "intervalwindow.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QIcon>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMetaObject>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

namespace {

class NumericTableWidgetItem : public QTableWidgetItem
{
public:
    explicit NumericTableWidgetItem(int value)
        : QTableWidgetItem(QString::number(value))
        , m_value(value)
    {
    }

    bool operator<(const QTableWidgetItem& other) const override
    {
        const auto* otherItem = dynamic_cast<const NumericTableWidgetItem*>(&other);
        if (otherItem) {
            return m_value < otherItem->m_value;
        }
        return text().toDouble() < other.text().toDouble();
    }

private:
    int m_value = 0;
};

QColor lineColorFromIndex(int index)
{
    static const QVector<QColor> colors = {
        QColor("#409EFF"), QColor("#67C23A"), QColor("#E6A23C"), QColor("#F56C6C")
    };
    return colors[index % colors.size()];
}

QFrame* createMetricCard(const QString& title, QLabel*& valueLabel, QWidget* parent)
{
    QFrame* card = new QFrame(parent);
    card->setObjectName("MetricCard");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font:600 12px 'Microsoft YaHei'; color:#5B6573; background:transparent;");
    valueLabel = new QLabel(QString::fromUtf8("--"), card);
    valueLabel->setStyleSheet("font:700 24px 'Microsoft YaHei'; color:#1F2D3D; background:transparent;");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addStretch();
    return card;
}

}

IntervalSeriesChartWidget::IntervalSeriesChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background: transparent;");
}

void IntervalSeriesChartWidget::setData(const QVector<QPair<int, int>>& data, const QString& title)
{
    m_data = data;
    m_title = title;
    update();
}

QVector<QPair<int, int>> IntervalSeriesChartWidget::data() const
{
    return m_data;
}

QString IntervalSeriesChartWidget::title() const
{
    return m_title;
}

void IntervalSeriesChartWidget::setZoomHintVisible(bool visible)
{
    m_showZoomHint = visible;
    update();
}

void IntervalSeriesChartWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
    painter.fillPath(bgPath, QColor(255, 255, 255, 210));
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1));
    painter.drawPath(bgPath);

    QRect drawRect = rect().adjusted(16, 16, -16, -20);
    painter.setPen(QColor("#1F2D3D"));
    painter.setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
    painter.drawText(QRect(drawRect.left(), drawRect.top(), drawRect.width(), 24),
                     Qt::AlignLeft | Qt::AlignVCenter, m_title);

    if (m_data.isEmpty()) {
        painter.setFont(QFont("Microsoft YaHei", 10));
        painter.setPen(QColor("#909399"));
        painter.drawText(drawRect, Qt::AlignCenter, QString::fromUtf8("暂无可展示的数据"));
        return;
    }

    QRect chartRect = drawRect.adjusted(0, 40, 0, 0);
    const int left = chartRect.left() + 24;
    const int right = chartRect.right() - 16;
    const int top = chartRect.top() + 8;
    const int bottom = chartRect.bottom() - 36;

    int maxValue = 1;
    for (const auto& point : m_data) {
        maxValue = std::max(maxValue, point.second);
    }

    painter.setPen(QPen(QColor("#DCDDE0"), 1));
    painter.drawLine(left, top, left, bottom);
    painter.drawLine(left, bottom, right, bottom);

    painter.setPen(QColor("#606266"));
    painter.setFont(QFont("Microsoft YaHei", 8));
    const int pointCount = static_cast<int>(m_data.size());
    const int maxLabels = width() < 420 ? 3 : (width() < 560 ? 4 : 6);
    const int labelStep = std::max(1, (pointCount + maxLabels - 1) / maxLabels);
    for (int i = 0; i < m_data.size(); ++i) {
        const double ratio = (m_data.size() == 1) ? 0.0 : static_cast<double>(i) / (m_data.size() - 1);
        const double x = left + (right - left) * ratio;
        if (i == 0 || i == m_data.size() - 1 || i % labelStep == 0) {
            painter.drawText(QRectF(x - 24, bottom + 8, 48, 16), Qt::AlignCenter, QString::number(m_data[i].first));
        }
    }

    QPainterPath path;
    for (int i = 0; i < m_data.size(); ++i) {
        const double x = left + (right - left) * ((m_data.size() == 1) ? 0.0 : static_cast<double>(i) / (m_data.size() - 1));
        const double y = bottom - (bottom - top - 8) * (static_cast<double>(m_data[i].second) / maxValue);
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(lineColorFromIndex(0), 2));
    painter.drawPath(path);
    painter.setBrush(lineColorFromIndex(0));
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < m_data.size(); ++i) {
        const double x = left + (right - left) * ((m_data.size() == 1) ? 0.0 : static_cast<double>(i) / (m_data.size() - 1));
        const double y = bottom - (bottom - top - 8) * (static_cast<double>(m_data[i].second) / maxValue);
        painter.drawEllipse(QPointF(x, y), 4, 4);
    }

    if (m_showZoomHint) {
        painter.setPen(QColor(110, 118, 129, 180));
        painter.setFont(QFont("Microsoft YaHei", 8));
        painter.drawText(QRect(drawRect.right() - 118, drawRect.top() + 18, 116, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::fromUtf8("单击图表可放大"));
    }
}

void IntervalSeriesChartWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit chartClicked();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

IntervalWindow::IntervalWindow(const QString& xmlPath, QWidget* parent)
    : QDialog(parent)
    , m_service(xmlPath)
{
    buildUi();
    setLoadingState(true, QString::fromUtf8("正在加载时间区间分析数据..."));
    QTimer::singleShot(0, this, [this]() {
        initializeDataAsync();
    });
}

void IntervalWindow::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    if (m_loadingOverlay && m_loadingOverlay->isVisible()) {
        m_loadingOverlay->setGeometry(rect().adjusted(18, 62, -18, -18));
    }
}

void IntervalWindow::refreshAnalysis()
{
    const int startYear = m_startYearCombo->currentData().toInt();
    const int endYear = m_endYearCombo->currentData().toInt();
    if (startYear > endYear) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("起始年份不能大于结束年份。"));
        return;
    }

    const IntervalSummary summary = m_service.querySummary(startYear, endYear);
    updateSummary(summary);
    m_yearChart->setData(m_service.querySeries(startYear, endYear),
                         QString::fromUtf8("%1 - %2 年发文趋势").arg(startYear).arg(endYear));
    const QVector<KeywordStat> hotKeywords = m_service.queryHotKeywords(startYear, endYear, 10);
    populateKeywordTable(hotKeywords);
    if (m_keywordEdit->text().trimmed().isEmpty()) {
        if (!hotKeywords.isEmpty()) {
            m_keywordEdit->setText(hotKeywords.first().word);
        } else {
            m_keywordEdit->clear();
        }
    }
    refreshKeywordTrend();
}

void IntervalWindow::refreshKeywordTrend()
{
    const int startYear = m_startYearCombo->currentData().toInt();
    const int endYear = m_endYearCombo->currentData().toInt();
    const QString keyword = m_keywordEdit->text().trimmed();
    if (keyword.isEmpty()) {
        m_keywordTrendChart->setData({}, QString::fromUtf8("关键词年度趋势"));
        m_keywordHintLabel->setText(QString::fromUtf8("输入一个关键词，可以查看它在所选时间区间内的逐年变化"));
        return;
    }

    const QVector<QPair<int, int>> trend = m_service.queryKeywordTrend(keyword, startYear, endYear);
    m_keywordTrendChart->setData(trend, QString::fromUtf8("关键词“%1”趋势").arg(keyword));

    int total = 0;
    for (const auto& point : trend) {
        total += point.second;
    }
    m_keywordHintLabel->setText(QString::fromUtf8("关键词“%1”在该区间累计出现 %2 次").arg(keyword).arg(total));
}

void IntervalWindow::buildUi()
{
    setWindowTitle(QString::fromUtf8("时间区间分析"));
    setMinimumSize(1180, 760);
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("WindowBg");
    setStyleSheet(
        "#WindowBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QLabel#IntervalTitle{font:700 24px 'Microsoft YaHei'; color:#1F2D3D; background:transparent; padding:10px 4px;}"
        "QGroupBox{font:600 13px 'Microsoft YaHei'; border:1px solid rgba(255,255,255,0.45); border-radius:8px; margin-top:12px; background-color:rgba(255,255,255,0.58);}"
        "QGroupBox::title{subcontrol-origin: margin; left:12px; padding:0 4px;}"
        "QComboBox,QLineEdit{font:12px 'Microsoft YaHei'; background-color:rgba(255,255,255,0.86); border:1px solid rgba(64,158,255,0.18); border-radius:6px; padding:6px 8px;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:8px 14px; font:12px 'Microsoft YaHei';}"
        "QPushButton:hover{background:#66B1FF;}"
        "QTableWidget{font:12px 'Microsoft YaHei'; background-color:rgba(255,255,255,0.72); alternate-background-color:rgba(247,249,252,0.7); border:1px solid rgba(255,255,255,0.35); border-radius:8px; gridline-color:rgba(228,231,237,0.65); selection-background-color:rgba(64,158,255,0.18); selection-color:#1F2D3D;}"
        "QHeaderView::section{background:rgba(242,246,252,0.86); padding:7px; border:none; border-bottom:1px solid rgba(235,238,245,0.85);}"
        "QFrame#MetricCard{background-color:rgba(255,255,255,0.76); border:1px solid rgba(255,255,255,0.42); border-radius:10px;}"
        );

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("时间区间分析"));
    titleLabel->setObjectName("IntervalTitle");
    root->addWidget(titleLabel);

    QGroupBox* controlBox = new QGroupBox(QString::fromUtf8("查询条件"), this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBox);
    m_startYearCombo = new QComboBox(this);
    m_endYearCombo = new QComboBox(this);
    QPushButton* queryButton = new QPushButton(QString::fromUtf8("开始分析"), this);
    controlLayout->addWidget(new QLabel(QString::fromUtf8("起始年份")));
    controlLayout->addWidget(m_startYearCombo);
    controlLayout->addWidget(new QLabel(QString::fromUtf8("结束年份")));
    controlLayout->addWidget(m_endYearCombo);
    controlLayout->addWidget(queryButton);
    controlLayout->addStretch();
    root->addWidget(controlBox);

    QWidget* metricsWidget = new QWidget(this);
    QGridLayout* metricsLayout = new QGridLayout(metricsWidget);
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setHorizontalSpacing(12);
    metricsLayout->setVerticalSpacing(12);
    metricsLayout->addWidget(createMetricCard(QString::fromUtf8("区间总发文量"), m_totalValueLabel, this), 0, 0);
    metricsLayout->addWidget(createMetricCard(QString::fromUtf8("年均发文量"), m_averageValueLabel, this), 0, 1);
    metricsLayout->addWidget(createMetricCard(QString::fromUtf8("峰值年份"), m_peakYearValueLabel, this), 0, 2);
    metricsLayout->addWidget(createMetricCard(QString::fromUtf8("峰值发文量"), m_peakCountValueLabel, this), 0, 3);
    root->addWidget(metricsWidget);

    m_yearChart = new IntervalSeriesChartWidget(this);
    m_keywordTrendChart = new IntervalSeriesChartWidget(this);

    QGroupBox* keywordBox = new QGroupBox(QString::fromUtf8("区间关键词分析"), this);
    QVBoxLayout* keywordLayout = new QVBoxLayout(keywordBox);
    QHBoxLayout* keywordControlLayout = new QHBoxLayout();
    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText(QString::fromUtf8("输入关键词查看该词在区间内的逐年趋势"));
    QPushButton* keywordTrendButton = new QPushButton(QString::fromUtf8("查看关键词趋势"), this);
    keywordControlLayout->addWidget(new QLabel(QString::fromUtf8("关键词")));
    keywordControlLayout->addWidget(m_keywordEdit, 1);
    keywordControlLayout->addWidget(keywordTrendButton);

    m_keywordHintLabel = new QLabel(QString::fromUtf8("区间热点词 TOP10"), this);
    m_keywordHintLabel->setStyleSheet("font:12px 'Microsoft YaHei'; color:#5B6573; background:transparent;");

    m_keywordTable = new QTableWidget(0, 3, this);
    m_keywordTable->setHorizontalHeaderLabels({QString::fromUtf8("排名"), QString::fromUtf8("关键词"), QString::fromUtf8("出现次数")});
    m_keywordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_keywordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_keywordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_keywordTable->setAlternatingRowColors(true);

    keywordLayout->addLayout(keywordControlLayout);
    keywordLayout->addWidget(m_keywordHintLabel);
    keywordLayout->addWidget(m_keywordTable, 1);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_yearChart);
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(keywordBox, 2);
    rightLayout->addWidget(m_keywordTrendChart, 3);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 5);
    root->addWidget(splitter, 1);

    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setAttribute(Qt::WA_StyledBackground, true);
    m_loadingOverlay->setStyleSheet("background-color: rgba(255, 255, 255, 0.24); border-radius: 12px;");
    m_loadingOverlay->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(m_loadingOverlay);
    overlayLayout->setContentsMargins(24, 24, 24, 24);
    overlayLayout->addStretch();
    m_loadingLabel = new QLabel(QString::fromUtf8("正在加载时间区间分析数据..."), m_loadingOverlay);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(
        "background-color: rgba(255,255,255,0.86);"
        "color: #1F2D3D;"
        "font: 600 15px 'Microsoft YaHei';"
        "padding: 16px 28px;"
        "border-radius: 12px;"
        "border: 1px solid rgba(255,255,255,0.4);");
    overlayLayout->addWidget(m_loadingLabel, 0, Qt::AlignCenter);
    overlayLayout->addStretch();

    connect(queryButton, &QPushButton::clicked, this, &IntervalWindow::refreshAnalysis);
    connect(keywordTrendButton, &QPushButton::clicked, this, &IntervalWindow::refreshKeywordTrend);
    connect(m_keywordEdit, &QLineEdit::returnPressed, this, &IntervalWindow::refreshKeywordTrend);
    connect(m_yearChart, &IntervalSeriesChartWidget::chartClicked, this, &IntervalWindow::showExpandedYearChart);
    connect(m_keywordTrendChart, &IntervalSeriesChartWidget::chartClicked, this, &IntervalWindow::showExpandedKeywordTrendChart);
}

void IntervalWindow::initializeDataAsync()
{
    if (m_initThread) {
        return;
    }

    m_initThread = QThread::create([this]() {
        const bool ok = m_service.ensureLoaded();
        const QString error = m_service.lastError();
        QMetaObject::invokeMethod(this, [this, ok, error]() {
            m_initThread = nullptr;
            setLoadingState(false);
            if (!ok) {
                QMessageBox::warning(this, QString::fromUtf8("数据不可用"), error);
                return;
            }

            populateYearSelectors();
            refreshAnalysis();
        }, Qt::QueuedConnection);
    });

    connect(m_initThread, &QThread::finished, m_initThread, &QObject::deleteLater);
    m_initThread->start();
}

void IntervalWindow::setLoadingState(bool loading, const QString& message)
{
    if (m_loadingOverlay) {
        m_loadingOverlay->setGeometry(rect().adjusted(18, 62, -18, -18));
        if (!message.isEmpty() && m_loadingLabel) {
            m_loadingLabel->setText(message);
        }
        m_loadingOverlay->setVisible(loading);
        if (loading) {
            m_loadingOverlay->raise();
        }
    }

    if (m_startYearCombo) {
        m_startYearCombo->setEnabled(!loading);
    }
    if (m_endYearCombo) {
        m_endYearCombo->setEnabled(!loading);
    }
    if (m_keywordEdit) {
        m_keywordEdit->setEnabled(!loading);
    }
}

void IntervalWindow::populateYearSelectors()
{
    m_startYearCombo->clear();
    m_endYearCombo->clear();
    const QList<int> years = m_service.availableYears();
    for (const int year : years) {
        m_startYearCombo->addItem(QString::number(year), year);
        m_endYearCombo->addItem(QString::number(year), year);
    }

    if (!years.isEmpty()) {
        m_startYearCombo->setCurrentIndex(0);
        m_endYearCombo->setCurrentIndex(m_endYearCombo->count() - 1);
    }
}

void IntervalWindow::populateKeywordTable(const QVector<KeywordStat>& keywords)
{
    m_keywordTable->setSortingEnabled(false);
    m_keywordTable->clearContents();
    m_keywordTable->setRowCount(keywords.size());
    for (int row = 0; row < keywords.size(); ++row) {
        m_keywordTable->setItem(row, 0, new NumericTableWidgetItem(row + 1));
        m_keywordTable->setItem(row, 1, new QTableWidgetItem(keywords[row].word));
        m_keywordTable->setItem(row, 2, new NumericTableWidgetItem(keywords[row].frequency));
    }
    m_keywordTable->setSortingEnabled(true);
    m_keywordTable->sortByColumn(0, Qt::AscendingOrder);
}

void IntervalWindow::updateSummary(const IntervalSummary& summary)
{
    m_totalValueLabel->setText(QString::number(summary.totalPapers));
    m_averageValueLabel->setText(QString::number(summary.averagePapers));
    m_peakYearValueLabel->setText(summary.peakYear > 0 ? QString::number(summary.peakYear) : QString::fromUtf8("--"));
    m_peakCountValueLabel->setText(QString::number(summary.peakCount));
}

void IntervalWindow::showExpandedYearChart()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("放大发文趋势"));
    dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
    dialog.resize(980, 620);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet("#ExpandedChartDialog{border-image:url(:/picture/bg.png);}");
    dialog.setObjectName("ExpandedChartDialog");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(16, 16, 16, 16);

    IntervalSeriesChartWidget* chart = new IntervalSeriesChartWidget(&dialog);
    chart->setMinimumHeight(560);
    chart->setData(m_yearChart->data(), m_yearChart->title());
    chart->setZoomHintVisible(false);
    layout->addWidget(chart);

    dialog.exec();
}

void IntervalWindow::showExpandedKeywordTrendChart()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("放大关键词趋势"));
    dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
    dialog.resize(980, 620);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setStyleSheet("#ExpandedChartDialog{border-image:url(:/picture/bg.png);}");
    dialog.setObjectName("ExpandedChartDialog");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(16, 16, 16, 16);

    IntervalSeriesChartWidget* chart = new IntervalSeriesChartWidget(&dialog);
    chart->setMinimumHeight(560);
    chart->setData(m_keywordTrendChart->data(), m_keywordTrendChart->title());
    chart->setZoomHintVisible(false);
    layout->addWidget(chart);

    dialog.exec();
}
