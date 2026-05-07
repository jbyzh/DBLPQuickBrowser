#include "precise.h"
#include "ui_precise.h"
#include "authorindexgraphcache.h"
#include "cooperationnet.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QFontMetricsF>
#include <QTextDocument>
#include <QTextOption>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>
#include <QSet>
#include <QMultiHash>
#include <QList>
#include <QEvent>
#include <QMouseEvent>
#include <QtConcurrent>
#include <algorithm>
#include <exception>

Precise::Precise(QWidget *parent, const QString& dataRootPath, EntryMode mode)
    : QWidget(parent)
    , ui(new Ui::Precise)
    , m_coopNet(new CooperationNet(this))
    , m_authorEdit(nullptr)
    , m_collabTable(nullptr)
    , m_barView(nullptr)
    , m_barScene(nullptr)
    , m_networkView(nullptr)
    , m_networkScene(nullptr)
    , m_zoomSlider(nullptr)
    , m_infoPanel(nullptr)
    , m_dataRootPath(dataRootPath.trimmed())
    , m_entryMode(mode)
    , m_f9AuthorEdit(nullptr)
    , m_f9RecommendTable(nullptr)
    , m_f9PageRankReady(false)
    , m_dynForceTimer(new QTimer(this))
    , m_barAnimTimer(new QTimer(this))
    , m_barAnimFrame(0)
    , m_dataLoaded(false)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/picture/book.jpeg"));
    if (m_entryMode == ModeF9) {
        setWindowTitle(QString::fromUtf8("智能推荐"));
    } else if (m_entryMode == ModeGraph) {
        setWindowTitle(QString::fromUtf8("学术协作图谱"));
    } else {
        setWindowTitle(QString::fromUtf8("合作关系查询"));
    }
    setFixedSize(700, 500);
    setObjectName("WindowBg1");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "#WindowBg1 { border-image: url(:/picture/bg.png); }"
        "QWidget#GlassPanel {"
        "    background-color: rgba(255, 255, 255, 0.76);"
        "    border: 1px solid rgba(255, 255, 255, 0.45);"
        "    border-radius: 8px;"
        "}"
        "QLineEdit, QTableWidget, QTextEdit, QGraphicsView {"
        "    background-color: rgba(255, 255, 255, 0.88);"
        "    border: 1px solid rgba(160, 180, 200, 0.75);"
        "    border-radius: 6px;"
        "    color: #1f2937;"
        "}"
        "QHeaderView::section {"
        "    background-color: rgba(245, 248, 252, 0.95);"
        "    color: #334155;"
        "    border: none;"
        "    border-bottom: 1px solid rgba(160, 180, 200, 0.7);"
        "    padding: 6px;"
        "    font-weight: 600;"
        "}"
        "QTableWidget {"
        "    gridline-color: rgba(210, 220, 230, 0.75);"
        "    alternate-background-color: rgba(246, 249, 252, 0.9);"
        "    selection-background-color: rgba(191, 219, 254, 0.95);"
        "    selection-color: #0f172a;"
        "}"
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.78);"
        "    color: #1f2937;"
        "    border: 1px solid rgba(170, 185, 205, 0.8);"
        "    border-radius: 6px;"
        "    padding: 6px 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.94);"
        "    border-color: #409eff;"
        "    color: #2563eb;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(235, 240, 246, 0.95);"
        "}"
        "QLabel { color: #1f2937; }"
        "QWidget#LoadingOverlay {"
        "    background-color: rgba(255, 255, 255, 0.60);"
        "    border-radius: 10px;"
        "}"
        "QLabel#LoadingLabel {"
        "    background-color: rgba(255, 255, 255, 0.92);"
        "    color: #1f2937;"
        "    border: 1px solid rgba(180, 195, 215, 0.85);"
        "    border-radius: 10px;"
        "    padding: 18px 28px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "}"
    );
    m_barAnimTimer->setInterval(35);
    m_dynForceTimer->setInterval(33);

    if (m_entryMode == ModeF9) {
        setupSmartRecommendUi();
    } else if (m_entryMode == ModeGraph) {
        setupAcademicGraphUi();
    } else {
        setupCooperationQueryUi();
    }

    createLoadingOverlay();

    QTimer::singleShot(0, this, [this]() {
        setLoadingState(true, QString::fromUtf8("正在加载合作关系数据，请稍候..."));
        QTimer::singleShot(30, this, [this]() {
            ensureDataLoaded();
            if (m_entryMode == ModeGraph && m_networkScene) {
                m_networkScene->clear();
                m_networkScene->addText(QString::fromUtf8("请输入目标学者以构建初始网络。系统支持双击节点执行“增量拓扑扩展”。"));
            }
            setLoadingState(false);
        });
    });
}

Precise::~Precise()
{
    ++m_graphRenderGen;
    cancelGraphRendering();
    if (m_f9PrWatcher) {
        m_f9PrWatcher->waitForFinished();
    }
    delete ui;
}

void Precise::closeEvent(QCloseEvent* event)
{
    if (m_isLoadingData || (m_f9PrWatcher && m_f9PrWatcher->future().isRunning())) {
        setLoadingState(true, QString::fromUtf8("正在处理当前请求，请稍候..."));
        event->ignore();
        return;
    }
    ++m_graphRenderGen;
    cancelGraphRendering();
    QWidget::closeEvent(event);
}

void Precise::cancelGraphRendering()
{
    stopDynamicGraphSimulation();
}

void Precise::stopDynamicGraphSimulation()
{
    if (m_dynForceTimer) {
        m_dynForceTimer->stop();
        disconnect(m_dynForceTimer, nullptr, this, nullptr);
    }
    ++m_graphRenderGen;
}

void Precise::startDynamicGraphSimulation()
{
    if (!m_dynForceTimer) {
        return;
    }
    disconnect(m_dynForceTimer, nullptr, this, nullptr);
    connect(m_dynForceTimer, &QTimer::timeout, this, &Precise::onDynamicGraphForceTick);
    m_dynSimTicks = 0;
    m_dynForceTimer->start();
}

void Precise::setupCooperationQueryUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* f2Box = new QWidget(this);
    f2Box->setObjectName("GlassPanel");
    f2Box->setAttribute(Qt::WA_StyledBackground, true);
    auto* f2Layout = new QVBoxLayout(f2Box);
    f2Layout->setContentsMargins(6, 6, 6, 6);
    f2Layout->setSpacing(6);

    auto* searchRow = new QHBoxLayout();
    searchRow->addWidget(new QLabel(QString::fromUtf8("作者名："), f2Box));
    m_authorEdit = new QLineEdit(f2Box);
    m_authorEdit->setPlaceholderText(QString::fromUtf8("输入作者名并点击搜索"));
    auto* searchBtn = new QPushButton(QString::fromUtf8("搜索"), f2Box);
    searchRow->addWidget(m_authorEdit, 1);
    searchRow->addWidget(searchBtn);
    f2Layout->addLayout(searchRow);

    m_collabTable = new QTableWidget(f2Box);
    m_collabTable->setColumnCount(2);
    m_collabTable->setHorizontalHeaderLabels({QString::fromUtf8("合作作者"), QString::fromUtf8("合作次数")});
    m_collabTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_collabTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_collabTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_collabTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_collabTable->setAlternatingRowColors(true);
    m_collabTable->setSortingEnabled(true);
    m_barScene = new QGraphicsScene(f2Box);
    m_barView = new QGraphicsView(m_barScene, f2Box);
    m_barView->setRenderHint(QPainter::Antialiasing);
    m_barView->setFrameShape(QFrame::NoFrame);
    auto* f2Split = new QSplitter(Qt::Vertical, f2Box);
    f2Split->addWidget(m_collabTable);
    f2Split->addWidget(m_barView);
    f2Split->setStretchFactor(0, 1);
    f2Split->setStretchFactor(1, 1);
    f2Split->setSizes({1, 1});
    f2Layout->addWidget(f2Split, 1);
    root->addWidget(f2Box, 1);

    connect(searchBtn, &QPushButton::clicked, this, &Precise::onSearchCollaborators);
    connect(m_authorEdit, &QLineEdit::returnPressed, this, &Precise::onSearchCollaborators);
    drawCollaboratorBarChart({});
}

void Precise::setupAcademicGraphUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* f7Box = new QWidget(this);
    f7Box->setObjectName("GlassPanel");
    f7Box->setAttribute(Qt::WA_StyledBackground, true);
    auto* f7Layout = new QVBoxLayout(f7Box);
    f7Layout->setContentsMargins(6, 6, 6, 6);
    f7Layout->setSpacing(6);

    auto* searchRow = new QHBoxLayout();
    searchRow->addWidget(new QLabel(QString::fromUtf8("目标学者："), f7Box));
    m_graphSearchEdit = new QLineEdit(f7Box);
    m_graphSearchEdit->setPlaceholderText(QString::fromUtf8("输入作者名，构建一跳合作子图（节点上限：50）"));
    searchRow->addWidget(m_graphSearchEdit, 1);
    auto* loadBtn = new QPushButton(QString::fromUtf8("加载子图"), f7Box);
    searchRow->addWidget(loadBtn);
    f7Layout->addLayout(searchRow);

    auto* ctrl = new QHBoxLayout();
    ctrl->setSpacing(6);
    ctrl->addWidget(new QLabel(QString::fromUtf8("布局：分步力导向"), f7Box));
    ctrl->addSpacing(24);
    auto* zoomLabel = new QLabel(QString::fromUtf8("缩放："), f7Box);
    zoomLabel->setStyleSheet(QStringLiteral("QLabel { margin-right: -10px; }"));
    ctrl->addWidget(zoomLabel);
    m_zoomSlider = new QSlider(Qt::Horizontal, f7Box);
    // 缂╂斁鍊嶇巼 = 婊戞潯鍊?/ 100锛堝湪 fitInView 涔嬪悗锛夛紱10~250 鈫?0.1脳~2.5脳锛岄粯璁?100 = 1.0脳
    m_zoomSlider->setRange(50, 300);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setFixedWidth(400);
    ctrl->addWidget(m_zoomSlider);
    auto* resetBtn = new QPushButton(QString::fromUtf8("重置"), f7Box);
    ctrl->addWidget(resetBtn);
    f7Layout->addLayout(ctrl);

    m_networkScene = new QGraphicsScene(f7Box);
    m_networkView = new QGraphicsView(m_networkScene, f7Box);
    m_networkView->setRenderHint(QPainter::Antialiasing);
    m_networkView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_networkView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_networkView->setFrameShape(QFrame::NoFrame);
    m_networkView->viewport()->installEventFilter(this);
    f7Layout->addWidget(m_networkView, 3);

    m_infoPanel = new QTextEdit(f7Box);
    m_infoPanel->setReadOnly(true);
    m_infoPanel->setFont(font());
    m_infoPanel->setAlignment(Qt::AlignCenter);
    m_infoPanel->setPlainText(
        QString::fromUtf8("单击：查看学者信息；双击：执行关联节点扩展。\n注：系统最多维护 100 个活跃节点，超出后会按拓扑距离自动裁剪。"));
    m_infoPanel->setMinimumHeight(120);
    f7Layout->addWidget(m_infoPanel, 1);
    root->addWidget(f7Box, 1);

    connect(loadBtn, &QPushButton::clicked, this, &Precise::onGraphSubgraphLoadClicked);
    connect(m_graphSearchEdit, &QLineEdit::returnPressed, this, &Precise::onGraphSubgraphLoadClicked);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &Precise::onZoomChanged);
    connect(resetBtn, &QPushButton::clicked, this, &Precise::onResetView);
    connect(m_networkScene, &QGraphicsScene::selectionChanged, this, &Precise::onNetworkSelectionChanged);
}

void Precise::setupSmartRecommendUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* f9Box = new QWidget(this);
    f9Box->setObjectName("GlassPanel");
    f9Box->setAttribute(Qt::WA_StyledBackground, true);
    auto* boxLayout = new QVBoxLayout(f9Box);
    boxLayout->setContentsMargins(10, 10, 10, 10);
    boxLayout->setSpacing(8);

    auto* queryRow = new QHBoxLayout();
    queryRow->addWidget(new QLabel(QString::fromUtf8("作者名："), this));
    m_f9AuthorEdit = new QLineEdit(this);
    m_f9AuthorEdit->setPlaceholderText(QString::fromUtf8("输入作者姓名后点击生成推荐"));
    queryRow->addWidget(m_f9AuthorEdit, 1);
    m_f9RunBtn = new QPushButton(QString::fromUtf8("生成推荐"), this);
    queryRow->addWidget(m_f9RunBtn);
    boxLayout->addLayout(queryRow);

    m_f9PrWatcher = new QFutureWatcher<QVector<QPair<QString, double>>>(this);
    connect(m_f9PrWatcher, &QFutureWatcherBase::finished, this, &Precise::onF9PageRankCacheFinished);

    m_f9RecommendTable = new QTableWidget(this);
    m_f9RecommendTable->setColumnCount(3);
    m_f9RecommendTable->setHorizontalHeaderLabels({QString::fromUtf8("推荐作者"), QString::fromUtf8("推荐分数"), QString::fromUtf8("影响力（PageRank）")});
    m_f9RecommendTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_f9RecommendTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_f9RecommendTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_f9RecommendTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_f9RecommendTable->setAlternatingRowColors(true);
    boxLayout->addWidget(m_f9RecommendTable, 1);
    root->addWidget(f9Box, 1);

    connect(m_f9RunBtn, &QPushButton::clicked, this, &Precise::onF9RecommendClicked);
    connect(m_f9AuthorEdit, &QLineEdit::returnPressed, this, &Precise::onF9RecommendClicked);

}

void Precise::createLoadingOverlay()
{
    if (m_loadingOverlay) {
        return;
    }
    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setObjectName("LoadingOverlay");
    m_loadingOverlay->setGeometry(rect());
    m_loadingOverlay->hide();
    auto* layout = new QVBoxLayout(m_loadingOverlay);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addStretch();
    m_loadingLabel = new QLabel(QString::fromUtf8("正在加载合作关系数据，请稍候..."), m_loadingOverlay);
    m_loadingLabel->setObjectName("LoadingLabel");
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_loadingLabel, 0, Qt::AlignCenter);
    layout->addStretch();
}

void Precise::setLoadingState(bool loading, const QString& message)
{
    m_isLoadingData = loading;
    if (!m_loadingOverlay) {
        return;
    }
    m_loadingOverlay->setGeometry(rect());
    if (!message.isEmpty() && m_loadingLabel) {
        m_loadingLabel->setText(message);
    }
    m_loadingOverlay->setVisible(loading);
    if (loading) {
        m_loadingOverlay->raise();
        setCursor(Qt::WaitCursor);
    } else {
        unsetCursor();
    }
}

void Precise::runAfterDataLoaded(const QString& message, const std::function<void()>& action)
{
    if (m_dataLoaded) {
        if (action) {
            action();
        }
        return;
    }
    if (m_isLoadingData) {
        return;
    }

    setLoadingState(true, message.isEmpty() ? QString::fromUtf8("正在加载合作关系数据，请稍候...") : message);
    QTimer::singleShot(0, this, [this, action]() {
        ensureDataLoaded();
        setLoadingState(false);
        if (action) {
            action();
        }
    });
}

void Precise::ensureF9PageRankCache()
{
    if (m_f9PageRankReady) {
        return;
    }
    if (!m_f9PrWatcher) {
        return;
    }
    if (m_f9PrWatcher->future().isRunning()) {
        return;
    }
    if (m_coopNet->authorCount() == 0) {
        m_f9PageRankByAuthor.clear();
        m_f9PageRankReady = true;
        if (m_f9RunBtn) {
            m_f9RunBtn->setEnabled(true);
        }
        return;
    }
    if (!m_authorIndexCanonPath.isEmpty()
        && AuthorIndexGraphCache::tryReusePageRank(m_authorIndexCanonPath, &m_f9PageRankByAuthor)) {
        m_f9PageRankReady = true;
        if (m_f9RunBtn) {
            m_f9RunBtn->setEnabled(true);
        }
        return;
    }
    setLoadingState(true, QString::fromUtf8("正在计算影响力缓存，请稍候..."));
    if (m_f9RunBtn) {
        m_f9RunBtn->setEnabled(false);
    }
    if (m_f9AuthorEdit) {
        m_f9AuthorEdit->setEnabled(false);
    }
    QHash<QString, QHash<QString, int>> adj = m_coopNet->adjacencyCopy();
    const QFuture<QVector<QPair<QString, double>>> fut = QtConcurrent::run(
        [adj = std::move(adj)]() { return CooperationNet::computePageRank(adj, 20, 0.85); });
    m_f9PrWatcher->setFuture(fut);
}

void Precise::onF9PageRankCacheFinished()
{
    if (!m_f9PrWatcher) {
        return;
    }
    const QVector<QPair<QString, double>> pr = m_f9PrWatcher->result();
    m_f9PageRankByAuthor.clear();
    for (const auto& p : pr) {
        m_f9PageRankByAuthor.insert(p.first, p.second);
    }
    m_f9PageRankReady = true;
    if (m_f9RunBtn) {
        m_f9RunBtn->setEnabled(true);
    }
    if (m_f9AuthorEdit) {
        m_f9AuthorEdit->setEnabled(true);
    }
    if (!m_authorIndexCanonPath.isEmpty()) {
        AuthorIndexGraphCache::rememberPageRank(m_authorIndexCanonPath, m_f9PageRankByAuthor);
    }
    setLoadingState(false);
}

void Precise::ensureDataLoaded()
{
    if (m_dataLoaded) {
        return;
    }
    initCooperationData();
    m_dataLoaded = true;
}

void Precise::initCooperationData()
{
    QStringList candidates;
    const QString cwd = QDir::currentPath();
    const QString appDir = QCoreApplication::applicationDirPath();
    if (!m_dataRootPath.isEmpty()) {
        candidates << (QDir(m_dataRootPath).absoluteFilePath("database/author"));
    }
    candidates << (cwd + "/database/author")
               << (appDir + "/database/author")
               << (QDir(appDir).absoluteFilePath("../database/author"))
               << (QDir(appDir).absoluteFilePath("../../database/author"));

    bool loaded = false;
    for (const QString& p : candidates) {
        if (!QDir(p).exists()) {
            continue;
        }
        if (AuthorIndexGraphCache::tryReuse(p, m_coopNet)) {
            m_authorIndexCanonPath = AuthorIndexGraphCache::canonicalAuthorDir(p);
            loaded = true;
            break;
        }
        if (m_coopNet->loadFromAuthorIndexDir(p)) {
            m_authorIndexCanonPath = AuthorIndexGraphCache::canonicalAuthorDir(p);
            AuthorIndexGraphCache::remember(p, m_coopNet->adjacencyCopy());
            loaded = true;
            break;
        }
    }
    if (!loaded) {
        m_authorIndexCanonPath.clear();
        m_coopNet->addPublication({"Alice", "Bob", "Cindy"}, "Graph Mining");
        m_coopNet->addPublication({"Alice", "David"}, "Ranking Study");
        m_coopNet->addPublication({"Bob", "Eva", "Frank"}, "Citation Analysis");
        m_coopNet->addPublication({"Cindy", "Eva"}, "UI Interaction");
        m_coopNet->addPublication({"David", "Frank", "Grace"}, "Data Search");
    }
}

void Precise::onSearchCollaborators()
{
    if (m_isLoadingData) {
        return;
    }
    const QString authorName = m_authorEdit ? m_authorEdit->text().trimmed() : QString();
    runAfterDataLoaded(QString::fromUtf8("正在加载合作关系数据，请稍候..."), [this, authorName]() {
        refreshCollaboratorResults(authorName);
    });
}

void Precise::refreshCollaboratorResults(const QString& authorName)
{
    if (!m_collabTable) {
        return;
    }
    QVector<QPair<QString, int>> ranked;
    const auto collab = m_coopNet->getCollaborators(authorName);
    ranked.reserve(collab.size());
    for (auto it = collab.constBegin(); it != collab.constEnd(); ++it) {
        ranked.push_back({it.key(), it.value()});
    }
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second == b.second) return a.first < b.first;
        return a.second > b.second;
    });

    m_collabTable->setSortingEnabled(false);
    m_collabTable->setRowCount(ranked.size());
    for (int i = 0; i < ranked.size(); ++i) {
        m_collabTable->setItem(i, 0, new QTableWidgetItem(ranked[i].first));
        m_collabTable->setItem(i, 1, new QTableWidgetItem(QString::number(ranked[i].second)));
    }
    m_collabTable->setSortingEnabled(true);
    m_collabTable->sortItems(1, Qt::DescendingOrder);

    QVector<QPair<QString, int>> topRows = ranked;
    if (topRows.size() > 12) {
        topRows.resize(12);
    }
    drawCollaboratorBarChart(topRows);
}

void Precise::drawCollaboratorBarChart(const QVector<QPair<QString, int>>& rankedRows)
{
    m_barAnimRows = rankedRows;
    m_barAnimFrame = 0;
    if (m_barAnimTimer->isActive()) {
        m_barAnimTimer->stop();
    }
    disconnect(m_barAnimTimer, nullptr, this, nullptr);
    connect(m_barAnimTimer, &QTimer::timeout, this, [this]() {
        ++m_barAnimFrame;
        const qreal progress = qMin(1.0, m_barAnimFrame / 10.0);
        renderCollaboratorBarChart(m_barAnimRows, progress);
        if (progress >= 1.0) {
            m_barAnimTimer->stop();
        }
    });
    renderCollaboratorBarChart(m_barAnimRows, 0.12);
    m_barAnimTimer->start();
}

void Precise::renderCollaboratorBarChart(const QVector<QPair<QString, int>>& rankedRows, qreal progress)
{
    if (!m_barScene || !m_barView) {
        return;
    }
    m_barScene->clear();

    const QRectF v = m_barView->viewport()->rect();
    const qreal w = qMax<qreal>(v.width(), 360.0);
    const qreal h = qMax<qreal>(v.height(), 220.0);
    const qreal mLeft = 8.0;
    const qreal labelColW = 30.0;
    const qreal mRight = 18.0;
    const qreal mTop = 28.0;
    const qreal mBottom = 58.0;
    m_barScene->addRect(0, 0, w, h, QPen(Qt::NoPen), QBrush(QColor(255, 255, 255, 150)));

    const qreal axisY0 = h - mBottom;
    const qreal axisX1 = w - mRight;
    const qreal axisY1 = mTop;
    const qreal axisX0 = mLeft + labelColW + 8.0;
    const qreal plotStartX = axisX0 + 6.0;
    const qreal plotW = axisX1 - plotStartX;
    const qreal plotH = axisY0 - axisY1;
    const qreal yMid = axisY1 + plotH * 0.5;

    auto* yTitle = m_barScene->addText(QString());
    {
        QFont f = yTitle->font();
        f.setPointSize(qMax(8, f.pointSize()));
        yTitle->setFont(f);
    }
    yTitle->document()->setDefaultTextOption(QTextOption(Qt::AlignHCenter));
    yTitle->setPlainText(QString::fromUtf8("合作次数"));
    yTitle->setTextWidth(labelColW);
    yTitle->adjustSize();
    const QRectF yBr = yTitle->boundingRect();
    yTitle->setPos(mLeft + (labelColW - yBr.width()) * 0.5, yMid - yBr.height() * 0.5);

    m_barScene->addLine(axisX0, axisY0, axisX1, axisY0, QPen(Qt::black, 1));
    m_barScene->addLine(axisX0, axisY0, axisX0, axisY1, QPen(Qt::black, 1));

    auto* xTitle = m_barScene->addText(QString::fromUtf8("合作作者"));
    {
        QFont f = xTitle->font();
        f.setPointSize(qMax(8, f.pointSize()));
        xTitle->setFont(f);
    }
    QRectF xBr = xTitle->boundingRect();
    xTitle->setPos((w - xBr.width()) * 0.5, axisY0 + 40);

    if (rankedRows.isEmpty()) {
        m_barView->fitInView(m_barScene->itemsBoundingRect(), Qt::KeepAspectRatio);
        return;
    }

    int maxV = 1;
    for (const auto& x : rankedRows) maxV = qMax(maxV, x.second);
    const qreal slotW = plotW / qMax(1, rankedRows.size());
    const qreal barW = slotW * 0.65;

    QFont nameFont = m_barScene->font();
    nameFont.setPointSize(qMax(7, nameFont.pointSize() - 1));
    const QFontMetricsF nameFm(nameFont);

    for (int i = 0; i < rankedRows.size(); ++i) {
        const qreal ratio = static_cast<qreal>(rankedRows[i].second) / maxV;
        const qreal bh = plotH * ratio * qBound(0.0, progress, 1.0);
        const qreal x = plotStartX + i * slotW + (slotW - barW) / 2.0;
        const qreal y = axisY0 - bh;
        m_barScene->addRect(x, y, barW, bh, QPen(Qt::NoPen), QBrush(QColor(64, 158, 255, 210)));
        if (progress >= 0.95) {
            auto* vText = m_barScene->addText(QString::number(rankedRows[i].second));
            QRectF tr = vText->boundingRect();
            const qreal cx = x + (barW - tr.width()) * 0.5;
            const qreal ty = y - tr.height() - 4;
            vText->setPos(cx, ty);

            const int nameMaxW = qMax(20, static_cast<int>(slotW * 0.92));
            const QString nameShown = nameFm.elidedText(rankedRows[i].first, Qt::ElideRight, nameMaxW);
            auto* nameItem = m_barScene->addText(nameShown);
            nameItem->setFont(nameFont);
            QRectF nr = nameItem->boundingRect();
            const qreal nameX = plotStartX + i * slotW + (slotW - nr.width()) * 0.5;
            nameItem->setPos(nameX, axisY0 + 4);
        }
    }
    m_barView->fitInView(m_barScene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void Precise::onZoomChanged(int value)
{
    if (!m_networkView || !m_networkScene) {
        return;
    }
    QRectF br = m_networkScene->itemsBoundingRect();
    if (br.isNull() || !br.isValid()) {
        br = QRectF(-220, -180, 440, 360);
    }
    ensureDataLoaded();
    m_networkView->resetTransform();
    m_networkView->fitInView(br, Qt::KeepAspectRatio);
    const qreal s = value / 100.0;
    m_networkView->scale(s, s);
}

void Precise::onResetView()
{
    if (!m_networkView || !m_networkScene) {
        return;
    }
    ensureDataLoaded();
    m_networkView->resetTransform();
    if (m_zoomSlider) {
        m_zoomSlider->setValue(100);
    }
    QRectF br = m_networkScene->itemsBoundingRect();
    if (br.isNull() || !br.isValid()) {
        br = QRectF(-220, -180, 440, 360);
    }
    m_networkView->fitInView(br, Qt::KeepAspectRatio);
}

namespace {

constexpr int kDynMaxInitialNeighbors = 49;
constexpr int kDynMaxCanvasNodes = 100;
constexpr qreal kDynGridCell = 90.0;
constexpr qreal kDynNodeRadius = 11.0;
constexpr qreal kDynRepulsion = 3800.0;
constexpr qreal kDynSpringK = 0.02;
constexpr qreal kDynIdealLen = 85.0;
constexpr qreal kDynDamping = 0.88;

} // namespace

QString Precise::dynEdgeKey(const QString& a, const QString& b)
{
    return a < b ? (a + "||" + b) : (b + "||" + a);
}

void Precise::clearDynamicGraphScene()
{
    stopDynamicGraphSimulation();
    m_dynNodes.clear();
    m_dynPos.clear();
    m_dynVel.clear();
    m_dynEdges.clear();
    m_dynNodeItems.clear();
    m_dynEdgeItems.clear();
    m_graphAnchorAuthor.clear();
    if (m_networkScene) {
        m_networkScene->clear();
    }
}

void Precise::rebuildDynamicEdges()
{
    m_dynEdges.clear();
    for (const QString& a : m_dynNodes) {
        const auto collab = m_coopNet->getCollaborators(a);
        for (auto it = collab.constBegin(); it != collab.constEnd(); ++it) {
            const QString& b = it.key();
            if (!m_dynNodes.contains(b) || a >= b) {
                continue;
            }
            m_dynEdges.push_back(qMakePair(a, b));
        }
    }
}

void Precise::rebuildDynamicGraphicsItems()
{
    if (!m_networkScene) {
        return;
    }
    m_networkScene->clear();
    m_dynNodeItems.clear();
    m_dynEdgeItems.clear();

    const qreal r = kDynNodeRadius;
    for (const QString& n : m_dynNodes) {
        auto* ellipse = new QGraphicsEllipseItem(QRectF(-r, -r, 2 * r, 2 * r));
        ellipse->setData(0, n);
        const bool isAnchor = (n == m_graphAnchorAuthor);
        ellipse->setBrush(QBrush(isAnchor ? QColor(218, 165, 32) : QColor(86, 156, 214)));
        ellipse->setPen(QPen(isAnchor ? QColor(120, 80, 10) : QColor(35, 85, 125), 1.2));
        ellipse->setFlag(QGraphicsItem::ItemIsSelectable, true);
        ellipse->setZValue(1.0);
        ellipse->setPos(m_dynPos.value(n));
        m_networkScene->addItem(ellipse);
        m_dynNodeItems.insert(n, ellipse);
    }

    for (const auto& e : m_dynEdges) {
        const QString k = dynEdgeKey(e.first, e.second);
        const QPointF pa = m_dynPos.value(e.first);
        const QPointF pb = m_dynPos.value(e.second);
        auto* line = m_networkScene->addLine(QLineF(pa, pb),
            QPen(QColor(65, 85, 110, 210), 1.35, Qt::SolidLine, Qt::RoundCap));
        line->setZValue(-1.0);
        m_dynEdgeItems.insert(k, line);
    }

    QRectF br = m_networkScene->itemsBoundingRect();
    if (!br.isNull()) {
        m_networkScene->setSceneRect(br.adjusted(-40, -40, 40, 40));
    }
    if (m_networkView) {
        m_networkView->resetTransform();
        m_networkView->fitInView(m_networkScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void Precise::syncDynamicGraphPositions()
{
    for (auto it = m_dynNodeItems.constBegin(); it != m_dynNodeItems.constEnd(); ++it) {
        it.value()->setPos(m_dynPos.value(it.key()));
    }
    for (auto it = m_dynEdgeItems.constBegin(); it != m_dynEdgeItems.constEnd(); ++it) {
        const QString k = it.key();
        const int sep = k.indexOf(QStringLiteral("||"));
        if (sep <= 0 || sep >= k.size() - 2) {
            continue;
        }
        const QString a = k.left(sep);
        const QString b = k.mid(sep + 2);
        it.value()->setLine(QLineF(m_dynPos.value(a), m_dynPos.value(b)));
    }
    if (m_networkView && m_dynSimTicks % 12 == 0) {
        QRectF br = m_networkScene->itemsBoundingRect();
        if (!br.isNull()) {
            m_networkScene->setSceneRect(br.adjusted(-40, -40, 40, 40));
        }
    }
}

void Precise::trimDynamicSubgraphToMaxNodes()
{
    if (m_graphAnchorAuthor.isEmpty() || m_dynNodes.size() <= kDynMaxCanvasNodes) {
        return;
    }
    const QPointF center = m_dynPos.value(m_graphAnchorAuthor, QPointF(0, 0));
    QVector<QPair<qreal, QString>> scored;
    scored.reserve(m_dynNodes.size());
    for (const QString& n : m_dynNodes) {
        if (n == m_graphAnchorAuthor) {
            continue;
        }
        const QPointF d = m_dynPos.value(n) - center;
        scored.push_back({d.x() * d.x() + d.y() * d.y(), n});
    }
    std::sort(scored.begin(), scored.end(), [](const auto& x, const auto& y) { return x.first > y.first; });
    int needRemove = m_dynNodes.size() - kDynMaxCanvasNodes;
    for (int i = 0; i < needRemove && i < scored.size(); ++i) {
        const QString rm = scored[i].second;
        m_dynNodes.remove(rm);
        m_dynPos.remove(rm);
        m_dynVel.remove(rm);
    }
}

void Precise::expandDynamicSubgraphFromAuthor(const QString& authorName)
{
    if (authorName.isEmpty() || !m_coopNet->hasAuthor(authorName)) {
        return;
    }
    const auto collab = m_coopNet->getCollaborators(authorName);
    for (auto it = collab.constBegin(); it != collab.constEnd(); ++it) {
        const QString& nb = it.key();
        if (!m_dynNodes.contains(nb)) {
            const QPointF base = m_dynPos.value(authorName, QPointF(0, 0));
            const qreal ang = QRandomGenerator::global()->generateDouble() * 6.28318530717958647692;
            const qreal rad = 40.0 + QRandomGenerator::global()->bounded(50);
            m_dynNodes.insert(nb);
            m_dynPos.insert(nb, base + QPointF(qCos(ang) * rad, qSin(ang) * rad));
            m_dynVel.insert(nb, QPointF(0, 0));
        }
    }
    trimDynamicSubgraphToMaxNodes();
    rebuildDynamicEdges();
    rebuildDynamicGraphicsItems();
}

void Precise::buildDynamicSubgraphFromAnchor(const QString& anchorName)
{
    const QString anchor = anchorName.trimmed();
    ensureDataLoaded();
    if (anchor.isEmpty() || !m_coopNet->hasAuthor(anchor)) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("未在合作网络中找到该作者，请检查姓名是否与数据一致。"));
        return;
    }

    stopDynamicGraphSimulation();
    m_dynNodes.clear();
    m_dynPos.clear();
    m_dynVel.clear();
    m_dynEdges.clear();
    m_dynNodeItems.clear();
    m_dynEdgeItems.clear();
    m_graphAnchorAuthor = anchor;

    QVector<QPair<int, QString>> ranked;
    const auto collab = m_coopNet->getCollaborators(anchor);
    ranked.reserve(collab.size());
    for (auto it = collab.constBegin(); it != collab.constEnd(); ++it) {
        ranked.push_back({it.value(), it.key()});
    }
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) {
            return a.first > b.first;
        }
        return a.second < b.second;
    });

    m_dynNodes.insert(anchor);
    m_dynPos.insert(anchor, QPointF(0, 0));
    m_dynVel.insert(anchor, QPointF(0, 0));

    const int take = qMin(kDynMaxInitialNeighbors, ranked.size());
    for (int i = 0; i < take; ++i) {
        const QString& nb = ranked[i].second;
        m_dynNodes.insert(nb);
        const qreal t = (i + 1.0) / qMax(1.0, take + 1.0);
        const qreal ang = t * 6.283185307179586;
        const qreal rad = 95.0 + (i % 5) * 8.0;
        m_dynPos.insert(nb, QPointF(qCos(ang) * rad, qSin(ang) * rad));
        m_dynVel.insert(nb, QPointF(0, 0));
    }

    rebuildDynamicEdges();
    rebuildDynamicGraphicsItems();
    startDynamicGraphSimulation();

    if (m_infoPanel) {
        m_infoPanel->setPlainText(QString::fromUtf8("已加载“") + anchor + QString::fromUtf8("”的一跳合作子图（")
            + QString::number(m_dynNodes.size()) + QString::fromUtf8(" 个节点）。双击节点可继续扩展。"));
    }
}

void Precise::dynamicGraphForceStep()
{
    if (m_dynNodes.isEmpty() || m_graphAnchorAuthor.isEmpty()) {
        return;
    }

    QStringList nodes;
    nodes.reserve(m_dynNodes.size());
    for (const QString& u : m_dynNodes) {
        nodes.append(u);
    }
    const int n = nodes.size();
    QHash<QString, QPointF> f;
    for (const QString& u : nodes) {
        f.insert(u, QPointF(0, 0));
    }

    const bool useGrid = (n >= 100);
    const qreal rep = kDynRepulsion;

    if (!useGrid) {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const QString& a = nodes[i];
                const QString& b = nodes[j];
                if (a == m_graphAnchorAuthor && b == m_graphAnchorAuthor) {
                    continue;
                }
                QPointF d = m_dynPos[a] - m_dynPos[b];
                qreal ds = d.x() * d.x() + d.y() * d.y();
                if (ds < 100.0) {
                    ds = 100.0;
                }
                const qreal dist = qSqrt(ds);
                const QPointF dir = d / dist;
                const QPointF rf = dir * (rep / ds);
                if (a != m_graphAnchorAuthor) {
                    f[a] += rf;
                }
                if (b != m_graphAnchorAuthor) {
                    f[b] -= rf;
                }
            }
        }
    } else {
        QMultiHash<QString, QString> grid;
        for (const QString& u : nodes) {
            const QPointF p = m_dynPos[u];
            const qint64 gx = static_cast<qint64>(qFloor(p.x() / kDynGridCell));
            const qint64 gy = static_cast<qint64>(qFloor(p.y() / kDynGridCell));
            const QString cell = QString::number(gx) + QLatin1Char(',') + QString::number(gy);
            grid.insert(cell, u);
        }
        for (const QString& a : nodes) {
            if (a == m_graphAnchorAuthor) {
                continue;
            }
            const QPointF pa = m_dynPos[a];
            const qint64 gxa = static_cast<qint64>(qFloor(pa.x() / kDynGridCell));
            const qint64 gya = static_cast<qint64>(qFloor(pa.y() / kDynGridCell));
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    const QString cell = QString::number(gxa + dx) + QLatin1Char(',') + QString::number(gya + dy);
                    const QList<QString> cellNodes = grid.values(cell);
                    for (const QString& b : cellNodes) {
                        if (b <= a) {
                            continue;
                        }
                        QPointF d = pa - m_dynPos[b];
                        qreal ds = d.x() * d.x() + d.y() * d.y();
                        if (ds < 100.0) {
                            ds = 100.0;
                        }
                        const qreal dist = qSqrt(ds);
                        const QPointF dir = d / dist;
                        const QPointF rf = dir * (rep / ds);
                        f[a] += rf;
                        if (b != m_graphAnchorAuthor) {
                            f[b] -= rf;
                        }
                    }
                }
            }
        }
    }

    for (const auto& e : m_dynEdges) {
        QPointF d = m_dynPos[e.second] - m_dynPos[e.first];
        qreal dist = qMax<qreal>(1.0, qSqrt(d.x() * d.x() + d.y() * d.y()));
        const QPointF fs = (d / dist) * ((dist - kDynIdealLen) * kDynSpringK);
        if (e.first != m_graphAnchorAuthor) {
            f[e.first] += fs;
        }
        if (e.second != m_graphAnchorAuthor) {
            f[e.second] -= fs;
        }
    }

    for (const QString& u : nodes) {
        if (u == m_graphAnchorAuthor) {
            continue;
        }
        QPointF v = (m_dynVel[u] + f[u]) * kDynDamping;
        const qreal vmax = 42.0;
        const qreal vl = qSqrt(v.x() * v.x() + v.y() * v.y());
        if (vl > vmax) {
            v *= (vmax / vl);
        }
        m_dynVel[u] = v;
        m_dynPos[u] += v;
    }

    m_dynPos[m_graphAnchorAuthor] = QPointF(0, 0);
    m_dynVel[m_graphAnchorAuthor] = QPointF(0, 0);
}

void Precise::onDynamicGraphForceTick()
{
    ++m_dynSimTicks;
    try {
        dynamicGraphForceStep();
        syncDynamicGraphPositions();
    } catch (...) {
        stopDynamicGraphSimulation();
        if (m_infoPanel) {
            m_infoPanel->setPlainText(QString::fromUtf8("力导向迭代发生异常，已暂停。"));
        }
    }
}

void Precise::onGraphSubgraphLoadClicked()
{
    if (m_isLoadingData) {
        return;
    }
    if (!m_graphSearchEdit) {
        return;
    }
    const QString authorName = m_graphSearchEdit->text();
    runAfterDataLoaded(QString::fromUtf8("正在加载合作关系数据，请稍候..."), [this, authorName]() {
        buildDynamicSubgraphFromAnchor(authorName);
    });
}

bool Precise::eventFilter(QObject* watched, QEvent* event)
{
    if (m_entryMode == ModeGraph && m_networkView && watched == m_networkView->viewport()
        && event->type() == QEvent::MouseButtonDblClick) {
        auto* me = static_cast<QMouseEvent*>(event);
        if (!m_networkScene || me->button() != Qt::LeftButton) {
            return QWidget::eventFilter(watched, event);
        }
        const QPointF scenePt = m_networkView->mapToScene(me->pos());
        QGraphicsItem* hit = m_networkScene->itemAt(scenePt, m_networkView->transform());
        while (hit && !hit->data(0).isValid()) {
            hit = hit->parentItem();
        }
        if (auto* ell = dynamic_cast<QGraphicsEllipseItem*>(hit)) {
            const QString name = ell->data(0).toString();
            if (!name.isEmpty()) {
                expandDynamicSubgraphFromAuthor(name);
                showAuthorInfo(name);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Precise::onNetworkSelectionChanged()
{
    if (!m_networkScene) return;
    const auto items = m_networkScene->selectedItems();
    if (items.isEmpty()) return;
    const QString name = items.first()->data(0).toString();
    if (!name.isEmpty()) showAuthorInfo(name);
}

void Precise::showAuthorInfo(const QString& authorName)
{
    if (!m_infoPanel) {
        return;
    }
    const auto collab = m_coopNet->getCollaborators(authorName);
    QVector<QPair<QString, int>> rows;
    rows.reserve(collab.size());
    for (auto it = collab.constBegin(); it != collab.constEnd(); ++it) {
        rows.push_back({it.key(), it.value()});
    }
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    m_infoPanel->setAlignment(Qt::AlignLeft);
    if (rows.isEmpty()) {
        const QString plain = QString::fromUtf8("作者：") + authorName + "\n"
            + QString::fromUtf8("合作作者数量：0\n")
            + QString::fromUtf8("Top 合作关系：\n  暂无合作作者\n");
        m_infoPanel->setPlainText(plain);
        return;
    }

    QString html;
    // 涓嶅湪 HTML 涓己鍒舵寚瀹氬瓧鍙凤紝浣跨敤淇℃伅闈㈡澘鑷韩瀛椾綋锛屼繚鎸佷笌鍏跺畠闈㈡澘涓€鑷淬€?    html += "<div>";
    html += "<p style='margin:0;'>作者：" + authorName.toHtmlEscaped() + "</p>";
    html += "<p style='margin:0;'>合作作者数量：" + QString::number(rows.size()) + "</p>";
    html += "<p style='margin:0 0 2px 0;'>Top 合作关系：</p>";
    // 浣跨敤鏈夊簭鍒楄〃锛岃嚜鍔ㄥ疄鐜扳€滈琛岀紪鍙?+ 鍚庣画鎹㈣涓庢枃瀛楀榻愨€濈殑鎮寕缂╄繘鏁堟灉銆?    html += "<ol style='margin:0; padding-left: 30px;'>";
    for (int i = 0; i < qMin(8, rows.size()); ++i) {
        html += "<li style='margin:0; padding:0;'>"
            + rows[i].first.toHtmlEscaped()
            + " (" + QString::number(rows[i].second) + ")</li>";
    }
    html += "</ol></div>";
    m_infoPanel->setHtml(html);
}

void Precise::onF9RecommendClicked()
{
    if (m_isLoadingData) {
        return;
    }
    runAfterDataLoaded(QString::fromUtf8("正在加载合作关系数据，请稍候..."), [this]() {
        if (!m_f9RecommendTable) {
            return;
        }
        const QString author = m_f9AuthorEdit ? m_f9AuthorEdit->text().trimmed() : QString();
        if (author.isEmpty()) {
            m_f9RecommendTable->setRowCount(0);
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请输入作者姓名后再生成推荐。"));
            return;
        }
        ensureF9PageRankCache();
        if (!m_f9PageRankReady) {
            if (m_f9PrWatcher && m_f9PrWatcher->future().isRunning()) {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("影响力（PageRank）正在后台计算，请稍后再点击生成推荐。"));
            }
            return;
        }
        const int nAuthors = m_coopNet->authorCount();
        const int topN = qMin(50, qMax(1, nAuthors));
        auto rec = m_coopNet->recommendAuthors(author, topN);
        if (rec.isEmpty()) {
            m_f9RecommendTable->setRowCount(0);
            QMessageBox::information(this, QString::fromUtf8("提示"),
                QString::fromUtf8("未找到该作者，或根据合作网络暂无推荐结果（该作者可能不在数据中，或暂无二级合作推荐）。"));
            return;
        }
        std::sort(rec.begin(), rec.end(), [this](const QPair<QString, int>& a, const QPair<QString, int>& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }
            const double pa = m_f9PageRankByAuthor.contains(a.first) ? m_f9PageRankByAuthor.value(a.first) : -1.0;
            const double pb = m_f9PageRankByAuthor.contains(b.first) ? m_f9PageRankByAuthor.value(b.first) : -1.0;
            if (pa != pb) {
                return pa > pb;
            }
            return a.first < b.first;
        });
        m_f9RecommendTable->setRowCount(rec.size());
        for (int i = 0; i < rec.size(); ++i) {
            const QString& name = rec[i].first;
            m_f9RecommendTable->setItem(i, 0, new QTableWidgetItem(name));
            m_f9RecommendTable->setItem(i, 1, new QTableWidgetItem(QString::number(rec[i].second)));
            QString prText;
            if (m_f9PageRankByAuthor.contains(name)) {
                prText = QString::number(m_f9PageRankByAuthor.value(name), 'e', 6);
            } else {
                prText = QString::fromUtf8("--");
            }
            m_f9RecommendTable->setItem(i, 2, new QTableWidgetItem(prText));
        }
    });
}

