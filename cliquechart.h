#ifndef CLIQUECHART_H
#define CLIQUECHART_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <vector>
#include <map>

class CliqueChart : public QWidget
{
    Q_OBJECT

public:
    explicit CliqueChart(QWidget *parent = nullptr, const std::vector<long long>& cliqueCounts = std::vector<long long>(), const std::map<int, int>& componentSizeDistribution = std::map<int, int>());
    ~CliqueChart();

private:
    void setupUI();
    void createCliqueChart();
    void createComponentChart();

    std::vector<long long> m_cliqueCounts;
    std::map<int, int> m_componentSizeDistribution;
    QtCharts::QChartView *m_cliqueChartView;
    QtCharts::QChartView *m_componentChartView;
};

#endif // CLIQUECHART_H