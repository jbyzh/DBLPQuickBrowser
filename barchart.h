#ifndef BARCHART_H
#define BARCHART_H

#include <QWidget>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QChartView>

namespace Ui {
class BarChart;
}

class BarChart : public QWidget
{
    Q_OBJECT

public:
    explicit BarChart(QWidget *parent = nullptr);
    ~BarChart();
    void setCliqueData(const QMap<int, qlonglong> &cliqueData);
    void setFullSubgraphData(const QMap<int, QString> &fullSubgraphData);
    
private:
    Ui::BarChart *ui;
    void createBarChart();
    void createLogBarChart(const QMap<int, double> &data);
    double parseScientificNotation(const QString &str);
    bool m_dragging = false;      // 是否正在拖动
    QPoint m_dragStartPosition;   // 拖动起点
    QMap<int, qlonglong> m_cliqueData; // 存储clique数据
    QMap<int, double> m_fullSubgraphData; // 存储完全子图数据
    bool m_useLogScale = false;   // 是否使用对数轴

protected:
    // 重写鼠标事件实现拖动
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // BARCHART_H
