#ifndef CLIQUE_H
#define CLIQUE_H

#include <QWidget>
#include <vector>
#include <map>

namespace Ui {
class Clique;
}

class CliqueAnalyse;
class BarChart;

class Clique : public QWidget
{
    Q_OBJECT

public:
    explicit Clique(QWidget *parent = nullptr);
    ~Clique();

public slots:
    void updateAnalysisResults(const std::vector<long long>& cliqueCounts, int totalCliqueCount, int maxCliqueSize, int totalAuthors, const std::map<int, int>& componentSizeDistribution, const std::vector<QString>& allCliqueCountsStr);
    void updateProgress(int value);
    void updateStatus(const QString& status);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Clique *ui;
    CliqueAnalyse* analyser;
    std::vector<long long> m_cliqueCounts;
    std::vector<QString> m_allCliqueCountsStr;  // 方案C：所有完全子图数量
    // std::map<int, int> m_componentSizeDistribution; // tableWidget_2已移除
    void setupTable();
    void displayResultsInTable(const std::vector<long long>& counts);  // 极大团
    void displayAllCliqueResultsInTable(const std::vector<QString>& counts);  // 所有完全子图
};

#endif // CLIQUE_H
