#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

class GraphManager {
public:
    static GraphManager& instance();
    
    void addAuthorsFromArticle(const std::vector<std::string>& authors);
    bool hasData() const;
    void clear();
    void internalClear();
    
    int getNodeCount() const;
    int getEdgeCount() const;
    std::string getAuthorName(int id) const;
    std::vector<int> getNeighbors(int u) const;
    std::vector<std::vector<int>> getComponents();
    void addPaperAuthorCount(int count);
    int getMaxAuthorsPerPaper() const;
    
    void saveToFile(const std::string& path);
    bool loadFromFile(const std::string& path);
    bool hasSavedData(const std::string& path) const;

private:
    GraphManager();
    ~GraphManager() = default;
    GraphManager(const GraphManager&) = delete;
    GraphManager& operator=(const GraphManager&) = delete;

    int addAuthor(const std::string& name);
    void addEdge(int id1, int id2);
    int getEdgeCountInternal() const;

    mutable std::mutex m_mutex;
    std::vector<std::string> m_authors;
    std::unordered_map<std::string, int> m_nameToId;
    std::vector<std::unordered_set<int>> m_adjacency;
    int m_maxAuthorsPerPaper = 0;
};

#endif // GRAPHMANAGER_H