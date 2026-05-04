#include "cliqueanalyse.h"
#include <QMessageBox>
#include <QSettings>
#include <algorithm>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QFileInfo>
#include <QThread>
#include <QRunnable>
#include <QThreadPool>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <set>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "collaborationgraph.h"
#include "GraphManager.h"
#include "DegeneracyAlgorithm.h"
#include <list>
#include <vector>

using namespace std;

CliqueAnalyse::CliqueAnalyse(QWidget *parent)
    : QWidget(parent) {
    graph = nullptr;
}

void CliqueAnalyse::startAnalysisWithAlgorithm()
{
    emit progressUpdated(0);
    buildAuthorGraph();

    if (graph) {
        emit progressUpdated(10);
        analyzeCliquesWithAlgorithm();
    }
}

void CliqueAnalyse::buildAuthorGraph()
{
    if (graph) { delete graph; graph = nullptr; }

    auto& gm = GraphManager::instance();
    
    if (!gm.hasData()) {
        QSettings settings("DBLPQuickBrowser", "Settings");
        QString dbPath = settings.value("databasePath", "").toString();
        
        bool loaded = false;
        
        if (!dbPath.isEmpty()) {
            QString graphPath = QFileInfo(dbPath).path() + "/database/graph";
            if (gm.loadFromFile(graphPath.toStdString())) {
                emit statusUpdated("已从数据库文件加载数据: " + graphPath);
                loaded = true;
            }
        }
        
        if (!loaded) {
            QString appDir = QCoreApplication::applicationDirPath();
            QStringList possiblePaths = {
                appDir + "/database/graph",
                appDir + "/../database/graph",
                appDir + "/../../database/graph",
                QDir::currentPath() + "/database/graph",
                QDir::currentPath() + "/../database/graph"
            };
            
            for (const QString& path : possiblePaths) {
                if (gm.loadFromFile(path.toStdString())) {
                    emit statusUpdated("已从数据库文件加载数据: " + path);
                    loaded = true;
                    break;
                }
            }
        }
        
        if (!loaded) {
            QMessageBox::warning(this, "错误",
                                 "未找到作者数据！\n\n"
                                 "请先在主界面完成XML文件解析。");
            return;
        }
    }
    
    graph = new CollaborationGraph();
    
    emit progressUpdated(5);
    
    for (int i = 0; i < gm.getNodeCount(); ++i) {
        graph->addAuthor(gm.getAuthorName(i));
    }
    
    emit progressUpdated(7);
    
    for (int i = 0; i < gm.getNodeCount(); ++i) {
        auto neighbors = gm.getNeighbors(i);
        for (int neighbor : neighbors) {
            if (neighbor > i) {
                graph->addEdge(gm.getAuthorName(i), gm.getAuthorName(neighbor));
            }
        }
    }
    
    graph->setMaxAuthorsPerPaper(gm.getMaxAuthorsPerPaper());
    
    emit progressUpdated(10);
}

CliqueAnalyse::~CliqueAnalyse()
{
    if (graph) {
        delete graph;
        graph = nullptr;
    }
}

QString CliqueAnalyse::calculateCombinationScientific(int n, int k) {
    if (k < 0 || k > n) return "0";
    if (k == 0 || k == n) return "1";
    if (k > n - k) k = n - k;

    double logResult = 0.0;
    for (int i = 0; i < k; ++i) {
        logResult += log10(n - i) - log10(i + 1);
    }

    int exponent = static_cast<int>(logResult);
    double mantissa = pow(10, logResult - exponent);

    QString result;
    if (exponent >= 0 && exponent < 3) {
        result = QString::number(pow(10, logResult), 'f', 0);
    } else {
        result = QString("%1e%2").arg(mantissa, 0, 'f', 2).arg(exponent);
    }

    return result;
}

QString CliqueAnalyse::addScientificStrings(const QString& a, const QString& b) {
    if (a == "0") return b;
    if (b == "0") return a;

    int e1 = a.indexOf('e');
    int e2 = b.indexOf('e');

    double m1, m2;
    int exp1, exp2;

    if (e1 == -1) {
        m1 = a.toDouble();
        exp1 = 0;
    } else {
        m1 = a.left(e1).toDouble();
        exp1 = a.mid(e1+1).toInt();
    }

    if (e2 == -1) {
        m2 = b.toDouble();
        exp2 = 0;
    } else {
        m2 = b.left(e2).toDouble();
        exp2 = b.mid(e2+1).toInt();
    }

    if (exp1 > exp2) {
        m2 *= pow(10, exp2 - exp1);
        exp2 = exp1;
    } else if (exp2 > exp1) {
        m1 *= pow(10, exp1 - exp2);
        exp1 = exp2;
    }

    double m = m1 + m2;
    int exp = exp1;

    if (m >= 10) {
        m /= 10;
        exp++;
    } else if (m < 1 && m != 0) {
        m *= 10;
        exp--;
    }

    QString result;
    if (exp >= 0 && exp < 3) {
        result = QString::number(pow(10, log10(m) + exp), 'f', 0);
    } else {
        result = QString("%1e%2").arg(m, 0, 'f', 2).arg(exp);
    }

    return result;
}

void CliqueAnalyse::analyzeCliquesWithAlgorithm()
{
    if (!graph) return;
    
    clock_t startTime = clock();
    
    emit progressUpdated(15);
    
    emit progressUpdated(30);
    CollaborationGraph::ComponentList components = graph->getComponents();
    
    long long totalCliqueCount = 0;
    int maxCliqueSize = 0;
    int totalAuthors = graph->getNodeCount();
    
    emit progressUpdated(40);
    
    std::map<int, long long> maximalCliqueCounts;
    std::map<int, QString> allCliqueCountsScientific;
    
    int componentIndex = 0;
    int totalComponents = components.size();
    
    for (const auto& component : components) {
        int componentSize = component.size();
        
        if (componentSize == 1) {
            maximalCliqueCounts[1]++;
            totalCliqueCount++;
            if (maxCliqueSize < 1) maxCliqueSize = 1;
            QString countStr = calculateCombinationScientific(1, 1);
            allCliqueCountsScientific[1] = addScientificStrings(allCliqueCountsScientific[1], countStr);
        } else if (componentSize == 2) {
            maximalCliqueCounts[2]++;
            totalCliqueCount++;
            if (maxCliqueSize < 2) maxCliqueSize = 2;
            
            for (int k = 1; k <= 2; ++k) {
                QString countStr = calculateCombinationScientific(2, k);
                allCliqueCountsScientific[k] = addScientificStrings(allCliqueCountsScientific[k], countStr);
            }
        } else if (componentSize >= 3) {
            // 构建邻接表，用于 DegeneracyAlgorithm
            std::map<int, int> globalToLocal;
            std::map<int, int> localToGlobal;
            for (size_t i = 0; i < component.size(); ++i) {
                globalToLocal[component[i]] = i;
                localToGlobal[i] = component[i];
            }
            
            std::vector<std::list<int>> adjacencyList(componentSize);
            for (int globalId : component) {
                int localId = globalToLocal[globalId];
                auto neighbors = graph->getNeighbors(globalId);
                for (int neighborGlobalId : neighbors) {
                    auto it = globalToLocal.find(neighborGlobalId);
                    if (it != globalToLocal.end()) {
                        adjacencyList[localId].push_back(it->second);
                    }
                }
            }
            
            // 使用 DegeneracyAlgorithm 计算极大团
            DegeneracyAlgorithm algo(adjacencyList);
            std::list<std::list<int>> cliques;
            algo.Run(cliques);
            
            for (const auto& clique : cliques) {
                int cliqueSize = clique.size();
                maximalCliqueCounts[cliqueSize]++;
                totalCliqueCount++;
                if (cliqueSize > maxCliqueSize) {
                    maxCliqueSize = cliqueSize;
                }
                
                for (int k = 1; k <= cliqueSize; ++k) {
                    QString countStr = calculateCombinationScientific(cliqueSize, k);
                    allCliqueCountsScientific[k] = addScientificStrings(allCliqueCountsScientific[k], countStr);
                }
            }
        }
        
        componentIndex++;
        if (componentIndex % 100 == 0) {
            int progress = 40 + (int)((double)componentIndex / totalComponents * 40);
            emit progressUpdated(progress);
        }
    }
    
    emit progressUpdated(85);
    
    emit progressUpdated(90);
    
    // 找到实际存在的最大阶数
    int actualMax = maxCliqueSize;
    if (!maximalCliqueCounts.empty()) {
        actualMax = maximalCliqueCounts.rbegin()->first;
    }
    
    // 构建结果
    std::vector<long long> maximalRes;
    maximalRes.resize(actualMax + 1, 0);
    for (auto& entry : maximalCliqueCounts) {
        maximalRes[entry.first] = entry.second;
    }
    
    std::vector<QString> allCliqueCountsStr;
    allCliqueCountsStr.resize(actualMax + 1, "0");
    for (int i = 1; i <= actualMax; i++) {
        QString val = allCliqueCountsScientific[i];
        if (val.isEmpty()) val = "0";
        allCliqueCountsStr[i] = val;
    }
    
    clock_t endTime = clock();
    [[maybe_unused]] double duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    
    emit analysisCompleted(maximalRes, (int)totalCliqueCount, actualMax, totalAuthors, std::map<int, int>(), allCliqueCountsStr);
}
