#ifndef COLLABORATIONGRAPH_H
#define COLLABORATIONGRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <memory>
#include <algorithm>
#include <climits>

class AuthorNode {
public:
    std::string name; // 作者名称
    int id;          // 作者编号
    std::unordered_set<int> neighbors; // 相邻作者的id集合

    AuthorNode(const std::string& name, int id) : name(name), id(id) {}
};

class CollaborationGraph
{
public:
    using ComponentList = std::vector<std::vector<int>>;
    
    CollaborationGraph();
    ~CollaborationGraph();

    // 添加作者到图中，返回作者编号
    int addAuthor(const std::string& name);

    // 添加合作关系（边）
    void addEdge(const std::string& name1, const std::string& name2);

    // 获取id为u的邻居
    std::set<int> getNeighbors(int u) const;

    // BFS计算连通分量
    ComponentList getComponents();

    // 查找连通分量中的所有极大团
    std::vector<std::vector<int>> findMaximalCliquesBronKerbosch(const std::vector<int>& component);

    // 获取节点数量
    int getNodeCount() const;
    
    // 获取边数量
    int getEdgeCount() const;
    
    // 根据ID获取作者名字
    std::string getAuthorName(int id) const;

    // 获取/设置论文最大作者数（用于超大团）
    void setMaxAuthorsPerPaper(int max) { m_maxAuthorsPerPaper = max; }
    int getMaxAuthorsPerPaper() const { return m_maxAuthorsPerPaper; }

    // 记录每篇论文作者数（用于统计大团）
    void addPaperAuthorCount(int count) {
        if (count > 10) m_paperAuthorCounts[count]++;
        if (count > m_maxAuthorsPerPaper) m_maxAuthorsPerPaper = count;
    }
    std::map<int, int> getPaperAuthorCounts() const { return m_paperAuthorCounts; }

private:
    std::vector<std::shared_ptr<AuthorNode>> nodes;
    std::unordered_map<std::string, int> name_to_id;
    int current_id;
    int m_maxAuthorsPerPaper = 0;
    std::map<int, int> m_paperAuthorCounts;  // key=作者数, value=论文数量

    // 小型连通分量的阈值
    static const int SMALL_COMPONENT_THRESHOLD = 50;
};

#endif // COLLABORATIONGRAPH_H
