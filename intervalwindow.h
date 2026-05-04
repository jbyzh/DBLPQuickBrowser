#ifndef INTERVALWINDOW_H
#define INTERVALWINDOW_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QTableWidget>
#include <QThread>
#include <QWidget>

#include "intervalanalysis.h"

class IntervalSeriesChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IntervalSeriesChartWidget(QWidget* parent = nullptr);
    void setData(const QVector<QPair<int, int>>& data, const QString& title);
    QVector<QPair<int, int>> data() const;
    QString title() const;
    void setZoomHintVisible(bool visible);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QVector<QPair<int, int>> m_data;
    QString m_title;
    bool m_showZoomHint = true;

signals:
    void chartClicked();
};

class IntervalWindow : public QDialog
{
    Q_OBJECT

public:
    explicit IntervalWindow(const QString& xmlPath, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void refreshAnalysis();
    void refreshKeywordTrend();
    void showExpandedYearChart();
    void showExpandedKeywordTrendChart();

private:
    void buildUi();
    void initializeDataAsync();
    void setLoadingState(bool loading, const QString& message = QString());
    void populateYearSelectors();
    void populateKeywordTable(const QVector<KeywordStat>& keywords);
    void updateSummary(const IntervalSummary& summary);

    IntervalAnalysisService m_service;

    QComboBox* m_startYearCombo = nullptr;
    QComboBox* m_endYearCombo = nullptr;
    QLabel* m_totalValueLabel = nullptr;
    QLabel* m_averageValueLabel = nullptr;
    QLabel* m_peakYearValueLabel = nullptr;
    QLabel* m_peakCountValueLabel = nullptr;
    QTableWidget* m_keywordTable = nullptr;
    QLineEdit* m_keywordEdit = nullptr;
    QLabel* m_keywordHintLabel = nullptr;
    IntervalSeriesChartWidget* m_yearChart = nullptr;
    IntervalSeriesChartWidget* m_keywordTrendChart = nullptr;
    QWidget* m_loadingOverlay = nullptr;
    QLabel* m_loadingLabel = nullptr;
    QPointer<QThread> m_initThread;
};

#endif
