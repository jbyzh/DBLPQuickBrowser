#ifndef ANALYTICSWINDOW_H
#define ANALYTICSWINDOW_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPointer>
#include <QTabWidget>
#include <QTableWidget>
#include <QThread>
#include <QWidget>

#include "analytics.h"

class BarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarChartWidget(QWidget* parent = nullptr);
    void setData(const QVector<QPair<QString, int>>& data, const QString& title);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<QPair<QString, int>> m_data;
    QString m_title;
};

class TrendChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrendChartWidget(QWidget* parent = nullptr);
    void setData(const QMap<QString, QVector<QPair<int, int>>>& data);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QMap<QString, QVector<QPair<int, int>>> m_data;
};

class AnalyticsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AnalyticsWindow(const QString& xmlPath, QWidget* parent = nullptr);
    void setInitialTab(int index);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void applyAuthorFilter();
    void refreshHotKeywords();
    void refreshTrends();

private:
    void buildUi();
    bool initializeData();
    void initializeDataAsync();
    void setLoadingState(bool loading, const QString& message = QString());
    void populateAuthorTable(const QVector<AuthorStat>& authors);
    void populateKeywordTable(const QVector<KeywordStat>& keywords);
    void populateYearList();

    AnalyticsService m_service;
    QTabWidget* m_tabs = nullptr;

    QLineEdit* m_authorSearchEdit = nullptr;
    QLabel* m_authorSummaryLabel = nullptr;
    QTableWidget* m_authorTable = nullptr;
    BarChartWidget* m_authorBarChart = nullptr;

    QComboBox* m_yearCombo = nullptr;
    QTableWidget* m_keywordTable = nullptr;
    QLabel* m_hotspotSummaryLabel = nullptr;
    QListWidget* m_yearList = nullptr;
    BarChartWidget* m_keywordBarChart = nullptr;
    TrendChartWidget* m_trendChart = nullptr;
    QWidget* m_loadingOverlay = nullptr;
    QLabel* m_loadingLabel = nullptr;
    QPointer<QThread> m_initThread;
};

#endif
