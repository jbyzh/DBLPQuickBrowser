#ifndef CLIQUEANALYSE_H
#define CLIQUEANALYSE_H

#include "CollaborationGraph.h"
#include "clique.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QProgressBar>
#include <QThreadPool>
#include <vector>

#include "DegeneracyAlgorithm.h"

class CliqueAnalyse:public QWidget
{
    Q_OBJECT
public:
    explicit CliqueAnalyse(QWidget *parent = nullptr);
    ~CliqueAnalyse();

public slots:
    void startAnalysisWithAlgorithm();

signals:
    void progressUpdated(int value);
    void statusUpdated(const QString& status);
    void analysisCompleted(const std::vector<long long>& cliqueCounts, int totalCliqueCount, int maxCliqueSize, int totalAuthors, const std::map<int, int>& componentSizeDistribution, const std::vector<QString>& allCliqueCountsStr);

private:
    void buildAuthorGraph();
    void analyzeCliquesWithAlgorithm();
    
    QString calculateCombinationScientific(int n, int k);
    QString addScientificStrings(const QString& a, const QString& b);

private:
    CollaborationGraph *graph;
    std::vector<long long> m_results;
    int m_totalCliqueCount;
    int m_maxCliqueSize;
    int m_totalAuthors;
    std::map<int, int> m_componentSizeDistribution;
    std::vector<QString> m_allCliqueCountsStr;
    
    // 存储最大团成员和作者映射
    std::vector<int> m_maxCliqueMembers;
    std::map<int, QString> m_authorMap;

public:
    // 获取最大团成员
    std::vector<int> getMaxCliqueMembers() const { return m_maxCliqueMembers; }
    // 获取作者映射
    std::map<int, QString> getAuthorMap() const { return m_authorMap; }
};

#endif // CLIQUEANALYSE_H
