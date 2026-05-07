#ifndef PRECISE_H
#define PRECISE_H

#include <QWidget>
#include <QCloseEvent>
#include <QHash>
#include <QPointF>
#include <QPair>
#include <QVector>
#include <QStringList>
#include <QSet>
#include <QFutureWatcher>
#include <QPushButton>
#include <functional>

class CooperationNet;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QLineEdit;
class QSlider;
class QSpinBox;
class QTableWidget;
class QTextEdit;
class QTimer;

namespace Ui {
class Precise;
}

class Precise : public QWidget
{
    Q_OBJECT

public:
    enum EntryMode {
        ModeF2 = 0,
        ModeF9,
        ModeGraph
    };

    explicit Precise(QWidget* parent = nullptr, const QString& dataRootPath = QString(), EntryMode mode = ModeF2);
    ~Precise();

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSearchCollaborators();
    void onZoomChanged(int value);
    void onResetView();
    void onNetworkSelectionChanged();
    void onF9RecommendClicked();
    void onF9PageRankCacheFinished();
    void onGraphSubgraphLoadClicked();
    void onDynamicGraphForceTick();

private:
    void setupCooperationQueryUi();  // F2
    void setupAcademicGraphUi();     // 学术协作图谱（动态子图）
    void setupSmartRecommendUi();    // F9
    void initCooperationData();
    void ensureDataLoaded();
    void refreshCollaboratorResults(const QString& authorName);
    void drawCollaboratorBarChart(const QVector<QPair<QString, int>>& rankedRows);
    void renderCollaboratorBarChart(const QVector<QPair<QString, int>>& rankedRows, qreal progress);
    void cancelGraphRendering();
    void showAuthorInfo(const QString& authorName);
    void ensureF9PageRankCache();
    void createLoadingOverlay();
    void setLoadingState(bool loading, const QString& message = QString());
    void runAfterDataLoaded(const QString& message, const std::function<void()>& action);

    // 动态协作子图（按需构建 + 分步力导向）
    void stopDynamicGraphSimulation();
    void startDynamicGraphSimulation();
    void clearDynamicGraphScene();
    void buildDynamicSubgraphFromAnchor(const QString& anchorName);
    void expandDynamicSubgraphFromAuthor(const QString& authorName);
    void trimDynamicSubgraphToMaxNodes();
    void rebuildDynamicEdges();
    void rebuildDynamicGraphicsItems();
    void dynamicGraphForceStep();
    void syncDynamicGraphPositions();
    static QString dynEdgeKey(const QString& a, const QString& b);

private:
    Ui::Precise* ui;
    CooperationNet* m_coopNet;
    QLineEdit* m_authorEdit;
    QTableWidget* m_collabTable;
    QGraphicsView* m_barView;
    QGraphicsScene* m_barScene;
    QGraphicsView* m_networkView;
    QGraphicsScene* m_networkScene;
    QSlider* m_zoomSlider;
    QTextEdit* m_infoPanel;
    QString m_dataRootPath;
    // 成功从磁盘加载的 author 索引目录规范路径；用于跨窗口图/PageRank 缓存键。
    QString m_authorIndexCanonPath;
    EntryMode m_entryMode;

    // F9 widgets
    QLineEdit* m_f9AuthorEdit;
    QTableWidget* m_f9RecommendTable;
    QPushButton* m_f9RunBtn = nullptr;
    QFutureWatcher<QVector<QPair<QString, double>>>* m_f9PrWatcher = nullptr;
    QHash<QString, double> m_f9PageRankByAuthor;
    bool m_f9PageRankReady = false;

    // 动态协作子图状态
    QLineEdit* m_graphSearchEdit = nullptr;
    QTimer* m_dynForceTimer = nullptr;
    QString m_graphAnchorAuthor;
    QSet<QString> m_dynNodes;
    QHash<QString, QPointF> m_dynPos;
    QHash<QString, QPointF> m_dynVel;
    QVector<QPair<QString, QString>> m_dynEdges;
    QHash<QString, QGraphicsEllipseItem*> m_dynNodeItems;
    QHash<QString, QGraphicsLineItem*> m_dynEdgeItems;
    int m_dynSimTicks = 0;
    int m_graphRenderGen = 0;

    QTimer* m_barAnimTimer;
    QVector<QPair<QString, int>> m_barAnimRows;
    int m_barAnimFrame;
    bool m_dataLoaded;
    bool m_isLoadingData = false;
    QWidget* m_loadingOverlay = nullptr;
    QLabel* m_loadingLabel = nullptr;
};

#endif // PRECISE_H
