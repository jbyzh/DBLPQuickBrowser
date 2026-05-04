#include "analyticswindow.h"

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QMetaObject>
#include <QPushButton>
#include <QSet>
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

QColor colorFromIndex(int index)
{
    static const QVector<QColor> colors = {
        QColor("#409EFF"), QColor("#67C23A"), QColor("#E6A23C"), QColor("#F56C6C"),
        QColor("#8E44AD"), QColor("#16A085"), QColor("#2C3E50"), QColor("#C0392B")
    };
    return colors[index % colors.size()];
}

}

BarChartWidget::BarChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background: transparent;");
}

void BarChartWidget::setData(const QVector<QPair<QString, int>>& data, const QString& title)
{
    m_data = data;
    m_title = title;
    update();
}

void BarChartWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
    painter.fillPath(bgPath, QColor(255, 255, 255, 210));
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1));
    painter.drawPath(bgPath);

    QRect drawRect = rect().adjusted(16, 16, -16, -24);
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

    QRect chartRect = drawRect.adjusted(0, 36, 0, 0);
    const int bottomMargin = 56;
    const int axisLeft = chartRect.left() + 40;
    const int axisBottom = chartRect.bottom() - bottomMargin;
    const int axisTop = chartRect.top() + 8;
    const int axisRight = chartRect.right() - 8;

    painter.setPen(QPen(QColor("#DCDDE0"), 1));
    painter.drawLine(axisLeft, axisTop, axisLeft, axisBottom);
    painter.drawLine(axisLeft, axisBottom, axisRight, axisBottom);

    int maxValue = 1;
    for (const auto& pair : m_data) {
        maxValue = std::max(maxValue, pair.second);
    }

    const int columnCount = static_cast<int>(m_data.size());
    const double totalWidth = axisRight - axisLeft;
    const double step = totalWidth / std::max(1, columnCount);
    const double barWidth = std::max(18.0, step * 0.55);

    painter.setFont(QFont("Microsoft YaHei", 8));
    for (int i = 0; i < columnCount; ++i) {
        const auto& item = m_data[i];
        const double ratio = static_cast<double>(item.second) / maxValue;
        const int barHeight = static_cast<int>((axisBottom - axisTop - 24) * ratio);
        const int x = static_cast<int>(axisLeft + i * step + (step - barWidth) / 2.0);
        const QRect barRect(x, axisBottom - barHeight, static_cast<int>(barWidth), barHeight);

        painter.fillRect(barRect, colorFromIndex(i));
        painter.setPen(QColor("#303133"));
        painter.drawText(QRect(x - 8, axisBottom - barHeight - 20, static_cast<int>(barWidth) + 16, 16),
                         Qt::AlignCenter, QString::number(item.second));

        painter.save();
        painter.translate(x + barWidth / 2.0, axisBottom + 8);
        painter.rotate(-32);
        painter.drawText(QRect(-42, 0, 84, 16), Qt::AlignCenter, item.first);
        painter.restore();
    }
}

TrendChartWidget::TrendChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(280);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background: transparent;");
}

void TrendChartWidget::setData(const QMap<QString, QVector<QPair<int, int>>>& data)
{
    m_data = data;
    update();
}

void TrendChartWidget::paintEvent(QPaintEvent* event)
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
                     Qt::AlignLeft | Qt::AlignVCenter, QString::fromUtf8("多年热点趋势对比"));

    if (m_data.isEmpty()) {
        painter.setFont(QFont("Microsoft YaHei", 10));
        painter.setPen(QColor("#909399"));
        painter.drawText(drawRect, Qt::AlignCenter, QString::fromUtf8("请选择至少两个年份"));
        return;
    }

    QRect chartRect = drawRect.adjusted(0, 40, 0, 0);
    const int left = chartRect.left() + 52;
    const int right = chartRect.right() - 16;
    const int top = chartRect.top() + 8;
    const int bottom = chartRect.bottom() - 40;

    QSet<int> yearSet;
    int maxValue = 1;
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        for (const auto& point : it.value()) {
            yearSet.insert(point.first);
            maxValue = std::max(maxValue, point.second);
        }
    }

    QList<int> years = yearSet.values();
    std::sort(years.begin(), years.end());
    if (years.size() < 2) {
        painter.setFont(QFont("Microsoft YaHei", 10));
        painter.setPen(QColor("#909399"));
        painter.drawText(drawRect, Qt::AlignCenter, QString::fromUtf8("请选择至少两个年份"));
        return;
    }

    painter.setPen(QPen(QColor("#DCDDE0"), 1));
    painter.drawLine(left, top, left, bottom);
    painter.drawLine(left, bottom, right, bottom);

    painter.setFont(QFont("Microsoft YaHei", 8));
    for (int i = 0; i < years.size(); ++i) {
        const double x = left + (right - left) * (static_cast<double>(i) / (years.size() - 1));
        painter.setPen(QPen(QColor("#E4E7ED"), 1, Qt::DashLine));
        painter.drawLine(QPointF(x, top), QPointF(x, bottom));
        painter.setPen(QColor("#606266"));
        painter.drawText(QRectF(x - 28, bottom + 8, 56, 16), Qt::AlignCenter, QString::number(years[i]));
    }

    int colorIndex = 0;
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it, ++colorIndex) {
        const QColor color = colorFromIndex(colorIndex);
        QPainterPath path;
        bool started = false;
        for (const auto& point : it.value()) {
            const int yearIndex = years.indexOf(point.first);
            if (yearIndex < 0) {
                continue;
            }
            const double x = left + (right - left) * (static_cast<double>(yearIndex) / (years.size() - 1));
            const double y = bottom - (bottom - top - 8) * (static_cast<double>(point.second) / maxValue);
            if (!started) {
                path.moveTo(x, y);
                started = true;
            } else {
                path.lineTo(x, y);
            }
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(x, y), 4, 4);
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(color, 2));
        painter.drawPath(path);
        painter.drawText(QRect(drawRect.right() - 150, drawRect.top() + colorIndex * 18, 140, 16),
                         Qt::AlignRight | Qt::AlignVCenter, it.key());
    }
}

AnalyticsWindow::AnalyticsWindow(const QString& xmlPath, QWidget* parent)
    : QDialog(parent)
    , m_service(xmlPath)
{
    buildUi();
    setLoadingState(true, QString::fromUtf8("正在加载作者统计与热点分析..."));
    QTimer::singleShot(0, this, [this]() {
        initializeDataAsync();
    });
}

void AnalyticsWindow::setInitialTab(int index)
{
    if (m_tabs && index >= 0 && index < m_tabs->count()) {
        m_tabs->setCurrentIndex(index);
    }
}

void AnalyticsWindow::buildUi()
{
    setWindowTitle(QString::fromUtf8("作者统计与热点分析"));
    setMinimumSize(1180, 760);
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("WindowBg");
    setStyleSheet(
        "#WindowBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QLabel#AnalyticsTitle{font:700 24px 'Microsoft YaHei'; color:#1F2D3D; background:transparent; padding:10px 4px;}"
        "QTabWidget::pane{border:1px solid rgba(255,255,255,0.38); background-color:rgba(255,255,255,0.52); top:-1px; border-radius:10px;}"
        "QTabBar::tab{background-color:rgba(255,255,255,0.45); color:#303133; padding:9px 18px; border:1px solid rgba(255,255,255,0.35); border-bottom:none; min-width:112px; border-top-left-radius:8px; border-top-right-radius:8px;}"
        "QTabBar::tab:selected{background-color:rgba(255,255,255,0.82); color:#409EFF;}"
        "QTabBar::tab:!selected{margin-top:2px;}"
        "QGroupBox{font:600 13px 'Microsoft YaHei'; border:1px solid rgba(255,255,255,0.45); border-radius:8px; margin-top:12px; background-color:rgba(255,255,255,0.58);}"
        "QGroupBox::title{subcontrol-origin: margin; left:12px; padding:0 4px;}"
        "QLineEdit,QComboBox{font:12px 'Microsoft YaHei'; background-color:rgba(255,255,255,0.86); border:1px solid rgba(64,158,255,0.18); border-radius:6px; padding:6px 8px;}"
        "QTableWidget,QListWidget{font:12px 'Microsoft YaHei'; background-color:rgba(255,255,255,0.72); alternate-background-color:rgba(247,249,252,0.7); border:1px solid rgba(255,255,255,0.35); border-radius:8px;}"
        "QTableWidget{gridline-color:rgba(228,231,237,0.65); selection-background-color:rgba(64,158,255,0.18); selection-color:#1F2D3D;}"
        "QListWidget::item{padding:4px 6px; border-radius:4px;}"
        "QListWidget::item:selected{background-color:rgba(64,158,255,0.14); color:#1F2D3D;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:8px 14px; font:12px 'Microsoft YaHei';}"
        "QPushButton:hover{background:#66B1FF;}"
        "QHeaderView::section{background:rgba(242,246,252,0.86); padding:7px; border:none; border-bottom:1px solid rgba(235,238,245,0.85);}"
        );

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("作者统计 / 年度热点分析"));
    titleLabel->setObjectName("AnalyticsTitle");

    m_tabs = new QTabWidget(this);

    QWidget* authorTab = new QWidget(this);
    QVBoxLayout* authorLayout = new QVBoxLayout(authorTab);
    QGroupBox* authorFilterBox = new QGroupBox(QString::fromUtf8("作者统计"));
    QHBoxLayout* authorFilterLayout = new QHBoxLayout(authorFilterBox);
    m_authorSearchEdit = new QLineEdit(this);
    m_authorSearchEdit->setPlaceholderText(QString::fromUtf8("输入作者名，支持部分匹配"));
    QPushButton* authorSearchButton = new QPushButton(QString::fromUtf8("搜索作者"));
    m_authorSummaryLabel = new QLabel(QString::fromUtf8("正在加载作者统计..."));
    m_authorSummaryLabel->setStyleSheet("color:#606266; background:transparent;");
    authorFilterLayout->addWidget(new QLabel(QString::fromUtf8("作者")));
    authorFilterLayout->addWidget(m_authorSearchEdit, 1);
    authorFilterLayout->addWidget(authorSearchButton);
    authorFilterLayout->addWidget(m_authorSummaryLabel, 1);

    m_authorTable = new QTableWidget(0, 3, this);
    m_authorTable->setHorizontalHeaderLabels({QString::fromUtf8("排名"), QString::fromUtf8("作者"), QString::fromUtf8("论文数")});
    m_authorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_authorTable->setSortingEnabled(true);
    m_authorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_authorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_authorTable->setAlternatingRowColors(true);

    m_authorBarChart = new BarChartWidget(this);
    QSplitter* authorSplitter = new QSplitter(Qt::Vertical, this);
    authorSplitter->addWidget(m_authorTable);
    authorSplitter->addWidget(m_authorBarChart);
    authorSplitter->setStretchFactor(0, 3);
    authorSplitter->setStretchFactor(1, 2);

    authorLayout->addWidget(authorFilterBox);
    authorLayout->addWidget(authorSplitter, 1);
    m_tabs->addTab(authorTab, QString::fromUtf8("作者统计"));

    QWidget* hotspotTab = new QWidget(this);
    QVBoxLayout* hotspotLayout = new QVBoxLayout(hotspotTab);

    QGroupBox* yearControlBox = new QGroupBox(QString::fromUtf8("年度热点"));
    QGridLayout* yearControlLayout = new QGridLayout(yearControlBox);
    m_yearCombo = new QComboBox(this);
    QPushButton* yearButton = new QPushButton(QString::fromUtf8("查看当年热点"));
    m_hotspotSummaryLabel = new QLabel(QString::fromUtf8("正在加载年度关键词..."));
    m_hotspotSummaryLabel->setStyleSheet("color:#606266; background:transparent;");
    yearControlLayout->addWidget(new QLabel(QString::fromUtf8("年份")), 0, 0);
    yearControlLayout->addWidget(m_yearCombo, 0, 1);
    yearControlLayout->addWidget(yearButton, 0, 2);
    yearControlLayout->addWidget(m_hotspotSummaryLabel, 0, 3);

    QGroupBox* compareBox = new QGroupBox(QString::fromUtf8("多年对比"));
    QHBoxLayout* compareLayout = new QHBoxLayout(compareBox);
    m_yearList = new QListWidget(this);
    m_yearList->setMaximumWidth(160);
    QPushButton* compareButton = new QPushButton(QString::fromUtf8("刷新趋势图"));
    compareLayout->addWidget(m_yearList);
    compareLayout->addWidget(compareButton);
    compareLayout->addStretch(1);

    m_keywordTable = new QTableWidget(0, 3, this);
    m_keywordTable->setHorizontalHeaderLabels({QString::fromUtf8("排名"), QString::fromUtf8("关键词"), QString::fromUtf8("出现次数")});
    m_keywordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_keywordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_keywordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_keywordTable->setAlternatingRowColors(true);

    m_keywordBarChart = new BarChartWidget(this);
    m_trendChart = new TrendChartWidget(this);

    QSplitter* hotspotSplitter = new QSplitter(Qt::Vertical, this);
    QWidget* upper = new QWidget(this);
    QHBoxLayout* upperLayout = new QHBoxLayout(upper);
    upperLayout->addWidget(m_keywordTable, 2);
    upperLayout->addWidget(m_keywordBarChart, 3);
    hotspotSplitter->addWidget(upper);
    hotspotSplitter->addWidget(m_trendChart);
    hotspotSplitter->setStretchFactor(0, 3);
    hotspotSplitter->setStretchFactor(1, 2);

    hotspotLayout->addWidget(yearControlBox);
    hotspotLayout->addWidget(compareBox);
    hotspotLayout->addWidget(hotspotSplitter, 1);
    m_tabs->addTab(hotspotTab, QString::fromUtf8("热点分析"));

    root->addWidget(titleLabel);
    root->addWidget(m_tabs, 1);

    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setAttribute(Qt::WA_StyledBackground, true);
    m_loadingOverlay->setStyleSheet("background-color: rgba(255, 255, 255, 0.24); border-radius: 12px;");
    m_loadingOverlay->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(m_loadingOverlay);
    overlayLayout->setContentsMargins(24, 24, 24, 24);
    overlayLayout->addStretch();
    m_loadingLabel = new QLabel(QString::fromUtf8("正在加载分析数据..."), m_loadingOverlay);
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

    connect(authorSearchButton, &QPushButton::clicked, this, &AnalyticsWindow::applyAuthorFilter);
    connect(m_authorSearchEdit, &QLineEdit::returnPressed, this, &AnalyticsWindow::applyAuthorFilter);
    connect(yearButton, &QPushButton::clicked, this, &AnalyticsWindow::refreshHotKeywords);
    connect(compareButton, &QPushButton::clicked, this, &AnalyticsWindow::refreshTrends);
}

bool AnalyticsWindow::initializeData()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const bool ok = m_service.ensureAnalytics();
    QApplication::restoreOverrideCursor();

    if (!ok) {
        QMessageBox::warning(this, QString::fromUtf8("数据不可用"), m_service.lastError());
        return false;
    }

    populateAuthorTable(m_service.topAuthors());
    populateYearList();
    refreshHotKeywords();
    refreshTrends();
    return true;
}

void AnalyticsWindow::initializeDataAsync()
{
    if (m_initThread) {
        return;
    }

    m_initThread = QThread::create([this]() {
        const bool ok = m_service.ensureAnalytics();
        const QString error = m_service.lastError();
        QMetaObject::invokeMethod(this, [this, ok, error]() {
            m_initThread = nullptr;
            setLoadingState(false);
            if (!ok) {
                QMessageBox::warning(this, QString::fromUtf8("数据不可用"), error);
                return;
            }

            populateAuthorTable(m_service.topAuthors());
            populateYearList();
            refreshHotKeywords();
            refreshTrends();
        }, Qt::QueuedConnection);
    });

    connect(m_initThread, &QThread::finished, m_initThread, &QObject::deleteLater);
    m_initThread->start();
}

void AnalyticsWindow::setLoadingState(bool loading, const QString& message)
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

    if (m_tabs) {
        m_tabs->setEnabled(!loading);
    }
}

void AnalyticsWindow::populateAuthorTable(const QVector<AuthorStat>& authors)
{
    m_authorTable->setSortingEnabled(false);
    m_authorTable->clearContents();
    m_authorTable->setRowCount(authors.size());
    for (int row = 0; row < authors.size(); ++row) {
        const AuthorStat& author = authors[row];
        m_authorTable->setItem(row, 0, new NumericTableWidgetItem(row + 1));
        m_authorTable->setItem(row, 1, new QTableWidgetItem(author.name));
        m_authorTable->setItem(row, 2, new NumericTableWidgetItem(author.paperCount));
    }
    m_authorTable->setSortingEnabled(true);
    m_authorTable->sortByColumn(0, Qt::AscendingOrder);

    QVector<QPair<QString, int>> chartData;
    const int chartCount = std::min(10, static_cast<int>(authors.size()));
    for (int i = 0; i < chartCount; ++i) {
        chartData.push_back(qMakePair(authors[i].name.section(' ', 0, 1), authors[i].paperCount));
    }
    m_authorBarChart->setData(chartData, QString::fromUtf8("TOP10 作者论文数量"));
    m_authorSummaryLabel->setText(QString::fromUtf8("当前展示 %1 条作者结果").arg(authors.size()));
}

void AnalyticsWindow::populateKeywordTable(const QVector<KeywordStat>& keywords)
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

void AnalyticsWindow::populateYearList()
{
    m_yearCombo->clear();
    m_yearList->clear();
    const QList<int> years = m_service.availableYears();
    if (years.isEmpty()) {
        return;
    }

    const int latestYear = years.last();
    const int preferredLatestYear = std::min(2025, latestYear);
    const int recentThreshold = preferredLatestYear - 4;
    for (auto it = years.crbegin(); it != years.crend(); ++it) {
        const int year = *it;
        m_yearCombo->addItem(QString::number(year), year);
        QListWidgetItem* item = new QListWidgetItem(QString::number(year), m_yearList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState((year >= recentThreshold && year <= preferredLatestYear) ? Qt::Checked : Qt::Unchecked);
    }

    int defaultIndex = m_yearCombo->findData(2025);
    if (defaultIndex < 0) {
        defaultIndex = m_yearCombo->count() - 1;
    }
    if (defaultIndex >= 0) {
        m_yearCombo->setCurrentIndex(defaultIndex);
    }
}

void AnalyticsWindow::applyAuthorFilter()
{
    const QString keyword = m_authorSearchEdit->text().trimmed();
    QVector<AuthorStat> authors = keyword.isEmpty() ? m_service.topAuthors() : m_service.searchAuthors(keyword);
    populateAuthorTable(authors);
    if (!keyword.isEmpty()) {
        m_authorSummaryLabel->setText(QString::fromUtf8("作者名包含“%1”的结果共 %2 条").arg(keyword).arg(authors.size()));
    }
}

void AnalyticsWindow::refreshHotKeywords()
{
    const int year = m_yearCombo->currentData().toInt();
    const QVector<KeywordStat> keywords = m_service.keywordsForYear(year);
    populateKeywordTable(keywords);

    QVector<QPair<QString, int>> chartData;
    for (const KeywordStat& keyword : keywords) {
        chartData.push_back(qMakePair(keyword.word, keyword.frequency));
    }
    m_keywordBarChart->setData(chartData, QString::fromUtf8("%1 年关键词 TOP10").arg(year));
    m_hotspotSummaryLabel->setText(QString::fromUtf8("%1 年共展示 %2 个高频词").arg(year).arg(keywords.size()));
}

void AnalyticsWindow::refreshTrends()
{
    QList<int> selectedYears;
    for (int row = 0; row < m_yearList->count(); ++row) {
        QListWidgetItem* item = m_yearList->item(row);
        if (item->checkState() == Qt::Checked) {
            selectedYears.push_back(item->text().toInt());
        }
    }
    std::sort(selectedYears.begin(), selectedYears.end());
    m_trendChart->setData(m_service.keywordTrends(selectedYears));
}

void AnalyticsWindow::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    if (m_loadingOverlay && m_loadingOverlay->isVisible()) {
        m_loadingOverlay->setGeometry(rect().adjusted(18, 62, -18, -18));
    }
}
