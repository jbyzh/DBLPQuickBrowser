#include "fuzzysearchdialog.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

#include <algorithm>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include <chrono>

namespace {

class ClickableChartView : public QChartView
{
public:
    explicit ClickableChartView(QChart* chart, QWidget* parent = nullptr)
        : QChartView(chart, parent)
    {
    }

    std::function<void()> onClick;

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        QChartView::mousePressEvent(event);
        if (event->button() == Qt::LeftButton && onClick) {
            onClick();
        }
    }
};

void showStyledMessageBox(QWidget* parent,
                          QMessageBox::Icon icon,
                          const QString& title,
                          const QString& text)
{
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setText(text);
    box.setWindowIcon(QIcon(":/picture/book.jpeg"));
    box.setStyleSheet(
        "QMessageBox{background-color:rgba(255,255,255,0.98);}"
        "QLabel{color:#1F2D3D; font:12px 'Microsoft YaHei'; min-width:140px; background:transparent;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:6px 14px; min-width:60px;}"
        "QPushButton:hover{background:#66B1FF;}"
    );
    box.exec();
}

void clearLayout(QLayout* layout)
{
    if (!layout) {
        return;
    }
    QLayoutItem* item = nullptr;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

QString sourceText(const search_module::SearchResult& result)
{
    if (!result.journal.isEmpty()) {
        return result.journal;
    }
    if (!result.venue.isEmpty()) {
        return result.venue;
    }
    return QString();
}

ClickableChartView* createYearChartView(const QVector<search_module::YearDistribution>& distribution,
                                        QWidget* parent,
                                        bool enlarged)
{
    QBarSet* set = new QBarSet(QString::fromUtf8("论文数量"));
    QStringList categories;
    int maxCount = 0;
    for (const auto& item : distribution) {
        *set << item.count;
        categories << QString::number(item.year);
        maxCount = std::max(maxCount, item.count);
    }

    QBarSeries* series = new QBarSeries();
    set->setColor(QColor("#409EFF"));
    series->append(set);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString::fromUtf8("年份分布"));
    chart->setTitleBrush(QBrush(QColor("#1F2D3D")));
    chart->setBackgroundBrush(QColor(255, 255, 255, 195));
    chart->setBackgroundPen(QPen(QColor(255, 255, 255, 110), 1));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setPlotAreaBackgroundBrush(QColor(255, 255, 255, 168));
    chart->setPlotAreaBackgroundPen(QPen(QColor(220, 223, 230, 160), 1));
    chart->legend()->setVisible(false);

    QCategoryAxis* axisX = new QCategoryAxis();
    const int categoryCount = categories.size();
    for (int i = 0; i < categoryCount; ++i) {
        axisX->append(categories[i], i);
    }
    axisX->setLabelsAngle(enlarged ? 90 : -45);
    axisX->setTickCount(categoryCount);
    axisX->setRange(-0.5, categoryCount - 0.5);
    axisX->setLabelsVisible(true);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setLabelsColor(QColor("#303133"));
    QFont axisFont("Microsoft YaHei", enlarged ? 9 : 8);
    axisX->setLabelsFont(axisFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    chart->setMargins(enlarged ? QMargins(10, 10, 10, 150) : QMargins(8, 8, 8, 85));

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, maxCount + std::max(1, maxCount / 8));
    axisY->setTickCount(6);
    axisY->setLabelFormat("%d");
    axisY->setLabelsColor(QColor("#303133"));
    axisY->setLabelsFont(axisFont);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ClickableChartView* view = new ClickableChartView(chart, parent);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");
    return view;
}

}

FuzzySearchDialog::FuzzySearchDialog(const QString& xmlPath, QWidget* parent)
    : QDialog(parent)
    , m_searcher(xmlPath)
    , m_xmlPath(xmlPath)
{
    buildUi();
}

void FuzzySearchDialog::buildUi()
{
    setWindowTitle(QString::fromUtf8("关键词模糊搜索"));
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    resize(900, 700);
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("FuzzyDialogBg");
    setStyleSheet(
        "#FuzzyDialogBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QLabel,QPushButton,QLineEdit,QListWidget{font:12px 'Microsoft YaHei';}"
        "QLabel{color:#1F2D3D; background:transparent;}"
        "QLineEdit{background-color:rgba(255,255,255,0.88); color:#1F2D3D; border:1px solid rgba(64,158,255,0.18); border-radius:6px; padding:6px 8px;}"
        "QListWidget{background-color:rgba(255,255,255,0.78); color:#1F2D3D; border:1px solid rgba(255,255,255,0.36); border-radius:8px; outline:0;}"
        "QListWidget::item{color:#1F2D3D; background:transparent; padding:10px 12px; border-bottom:1px solid rgba(31,45,61,0.08);}"
        "QListWidget::item:selected{background-color:rgba(64,158,255,0.22); color:#0F172A;}"
        "QListWidget::item:hover{background-color:rgba(255,255,255,0.35); color:#1F2D3D;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:8px 14px;}"
        "QPushButton:hover{background:#66B1FF;}"
    );

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    QLabel* title = new QLabel(QString::fromUtf8("关键词模糊搜索"), this);
    title->setStyleSheet("font:700 22px 'Microsoft YaHei'; padding:6px 2px;");
    root->addWidget(title);

    QHBoxLayout* top = new QHBoxLayout();
    top->addWidget(new QLabel(QString::fromUtf8("关键词"), this));
    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText(QString::fromUtf8("请输入标题关键词"));
    top->addWidget(m_keywordEdit, 1);
    m_searchButton = new QPushButton(QString::fromUtf8("搜索"), this);
    top->addWidget(m_searchButton);
    root->addLayout(top);

    QVBoxLayout* content = new QVBoxLayout();
    m_resultList = new QListWidget(this);
    m_resultList->setWordWrap(true);
    content->addWidget(m_resultList, 3);

    QHBoxLayout* pager = new QHBoxLayout();
    m_pageLabel = new QLabel(QString::fromUtf8("第 0 / 0 页"), this);
    pager->addWidget(m_pageLabel);
    m_statusLabel = new QLabel(QString(), this);
    m_statusLabel->setStyleSheet("color:#1F2D3D;");
    pager->addWidget(m_statusLabel);
    pager->addStretch(1);
    m_prevButton = new QPushButton(QString::fromUtf8("上一页"), this);
    m_nextButton = new QPushButton(QString::fromUtf8("下一页"), this);
    pager->addWidget(m_prevButton);
    pager->addWidget(m_nextButton);
    content->addLayout(pager);

    m_chartContainer = new QWidget(this);
    m_chartContainer->setMinimumHeight(330);
    m_chartContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_chartContainer->setStyleSheet("background-color:rgba(255,255,255,0.24); border:1px solid rgba(255,255,255,0.35); border-radius:10px;");
    content->addWidget(m_chartContainer, 2);
    root->addLayout(content, 1);

    m_searchPollTimer = new QTimer(this);
    m_searchPollTimer->setInterval(120);

    connect(m_searchButton, &QPushButton::clicked, this, &FuzzySearchDialog::executeSearch);
    connect(m_keywordEdit, &QLineEdit::returnPressed, this, &FuzzySearchDialog::executeSearch);
    connect(m_resultList, &QListWidget::itemDoubleClicked, this, &FuzzySearchDialog::showResultDetail);
    connect(m_prevButton, &QPushButton::clicked, this, &FuzzySearchDialog::previousPage);
    connect(m_nextButton, &QPushButton::clicked, this, &FuzzySearchDialog::nextPage);
    connect(m_searchPollTimer, &QTimer::timeout, this, &FuzzySearchDialog::pollSearchResult);

    updatePage();
    updateYearChart();
}

void FuzzySearchDialog::executeSearch()
{
    if (m_pendingSearch.valid()) {
        return;
    }

    m_currentKeyword = m_keywordEdit->text().trimmed();
    if (m_currentKeyword.isEmpty()) {
        showStyledMessageBox(this, QMessageBox::Warning, QString::fromUtf8("提示"), QString::fromUtf8("请输入关键词。"));
        return;
    }

    m_searchButton->setEnabled(false);
    m_keywordEdit->setEnabled(false);
    m_prevButton->setEnabled(false);
    m_nextButton->setEnabled(false);
    m_statusLabel->setText(QString::fromUtf8("正在搜索，请稍候..."));
    m_resultList->clear();
    QListWidgetItem* loadingItem = new QListWidgetItem(QString::fromUtf8("正在搜索，请稍候..."), m_resultList);
    loadingItem->setFlags(loadingItem->flags() & ~Qt::ItemIsSelectable);

    const QString xmlPath = m_xmlPath;
    const QString keyword = m_currentKeyword;
    m_pendingSearch = std::async(std::launch::async, [xmlPath, keyword]() {
        search_module::Searcher searcher(xmlPath);
        QVector<search_module::SearchResult> results = searcher.fuzzySearch(keyword);
        for (search_module::SearchResult& result : results) {
            searcher.readPaperDetails(result);
        }
        return results;
    });
    m_searchPollTimer->start();
}

void FuzzySearchDialog::updatePage()
{
    m_resultList->clear();

    const int totalPages = m_results.isEmpty() ? 0 : ((m_results.size() + m_itemsPerPage - 1) / m_itemsPerPage);
    const int start = m_currentPage * m_itemsPerPage;
    const int end = std::min(start + m_itemsPerPage, static_cast<int>(m_results.size()));

    if (m_results.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8("未找到相关结果"), m_resultList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }

    for (int i = start; i < end; ++i) {
        QListWidgetItem* item = new QListWidgetItem(resultSummaryText(m_results[i]), m_resultList);
        item->setData(Qt::UserRole, i);
        item->setSizeHint(QSize(item->sizeHint().width(), 62));
    }

    m_pageLabel->setText(QString::fromUtf8("第 %1 / %2 页")
                             .arg(totalPages == 0 ? 0 : m_currentPage + 1)
                             .arg(totalPages));
    m_prevButton->setEnabled(m_currentPage > 0);
    m_nextButton->setEnabled(totalPages > 0 && m_currentPage < totalPages - 1);
}

QString FuzzySearchDialog::resultSummaryText(const search_module::SearchResult& result) const
{
    QStringList parts;
    parts << result.title;
    QStringList meta;
    if (!result.year.isEmpty()) {
        meta << QString::fromUtf8("年份: %1").arg(result.year);
    }
    const QString source = sourceText(result);
    if (!source.isEmpty()) {
        meta << QString::fromUtf8("来源: %1").arg(source);
    }
    if (!meta.isEmpty()) {
        parts << meta.join("    ");
    }
    return parts.join("\n");
}

void FuzzySearchDialog::updateYearChart()
{
    QLayout* existing = m_chartContainer->layout();
    if (existing) {
        clearLayout(existing);
        delete existing;
    }

    QVBoxLayout* layout = new QVBoxLayout(m_chartContainer);
    layout->setContentsMargins(8, 8, 8, 8);

    const QVector<search_module::YearDistribution> distribution = m_searcher.getYearDistribution(m_results);
    if (distribution.isEmpty()) {
        QLabel* label = new QLabel(QString::fromUtf8("暂无可展示的年份分布"), m_chartContainer);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color:#1F2D3D;");
        layout->addWidget(label, 1);
        return;
    }

    ClickableChartView* view = createYearChartView(distribution, m_chartContainer, false);
    view->onClick = [this, distribution]() {
        QDialog dialog(this);
        dialog.setWindowTitle(QString::fromUtf8("年份分布 - 放大查看"));
        dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
        dialog.resize(1480, 860);
        dialog.setAttribute(Qt::WA_StyledBackground, true);
        dialog.setObjectName("FuzzyChartZoomBg");
        dialog.setStyleSheet(
            "#FuzzyChartZoomBg{border-image:url(:/picture/bg.png);}"
            "QDialog{background:transparent;}"
        );

        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
        ClickableChartView* zoomView = createYearChartView(distribution, &dialog, true);
        zoomView->onClick = nullptr;
        dialogLayout->addWidget(zoomView, 1);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        dialogLayout->addWidget(buttonBox);
        dialog.exec();
    };
    layout->addWidget(view, 1);

    QLabel* hint = new QLabel(QString::fromUtf8("单击图表可放大"), m_chartContainer);
    hint->setAlignment(Qt::AlignRight);
    hint->setStyleSheet("color:rgba(31,45,61,0.75); font:11px 'Microsoft YaHei'; background:transparent;");
    layout->addWidget(hint);
}

void FuzzySearchDialog::showResultDetail(QListWidgetItem* item)
{
    const int index = item->data(Qt::UserRole).toInt();
    if (index < 0 || index >= m_results.size()) {
        return;
    }
    showDetailDialog(m_results[index]);
}

void FuzzySearchDialog::showDetailDialog(search_module::SearchResult result)
{
    m_searcher.readPaperDetails(result);

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("论文详情"));
    dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
    dialog.resize(720, 540);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setObjectName("FuzzyDetailDialogBg");
    dialog.setStyleSheet(
        "#FuzzyDetailDialogBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QTextEdit{background-color:rgba(255,255,255,0.84); color:#1F2D3D; border:1px solid rgba(255,255,255,0.35); border-radius:8px; font:12px 'Microsoft YaHei';}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setReadOnly(true);

    QString detail;
    if (!result.title.isEmpty()) detail += QString::fromUtf8("<h3>标题: %1</h3>").arg(m_searcher.highlightKeyword(result.title, m_currentKeyword));
    if (!result.authors.isEmpty()) detail += QString::fromUtf8("<p>作者: %1</p>").arg(result.authors);
    if (!result.year.isEmpty()) detail += QString::fromUtf8("<p>年份: %1</p>").arg(result.year);
    if (!result.venue.isEmpty()) detail += QString::fromUtf8("<p>会议/书名: %1</p>").arg(result.venue);
    if (!result.journal.isEmpty()) detail += QString::fromUtf8("<p>期刊: %1</p>").arg(result.journal);
    if (!result.volume.isEmpty()) detail += QString::fromUtf8("<p>卷号: %1</p>").arg(result.volume);
    if (!result.number.isEmpty()) detail += QString::fromUtf8("<p>期号: %1</p>").arg(result.number);
    if (!result.pages.isEmpty()) detail += QString::fromUtf8("<p>页码: %1</p>").arg(result.pages);
    if (!result.doi.isEmpty()) detail += QString::fromUtf8("<p>链接/DOI: %1</p>").arg(result.doi);
    textEdit->setHtml(detail);
    layout->addWidget(textEdit, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);
    dialog.exec();
}

void FuzzySearchDialog::previousPage()
{
    if (m_currentPage > 0) {
        --m_currentPage;
        updatePage();
    }
}

void FuzzySearchDialog::nextPage()
{
    const int totalPages = m_results.isEmpty() ? 0 : ((m_results.size() + m_itemsPerPage - 1) / m_itemsPerPage);
    if (m_currentPage + 1 < totalPages) {
        ++m_currentPage;
        updatePage();
    }
}

void FuzzySearchDialog::pollSearchResult()
{
    if (!m_pendingSearch.valid()) {
        m_searchPollTimer->stop();
        return;
    }

    const auto status = m_pendingSearch.wait_for(std::chrono::milliseconds(0));
    if (status != std::future_status::ready) {
        return;
    }

    m_searchPollTimer->stop();
    m_results = m_pendingSearch.get();
    m_currentPage = 0;
    updatePage();
    updateYearChart();

    m_searchButton->setEnabled(true);
    m_keywordEdit->setEnabled(true);
    m_statusLabel->setText(QString::fromUtf8("共 %1 条结果").arg(m_results.size()));
}
