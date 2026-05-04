#include "GraphManager.h"
#include <QDebug>
#include <queue>
#include <fstream>
#include <sstream>

GraphManager::GraphManager() {
}

GraphManager& GraphManager::instance() {
    static GraphManager instance;
    return instance;
}

bool GraphManager::hasData() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_authors.empty();
}

void GraphManager::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    internalClear();
}

void GraphManager::internalClear() {
    m_authors.clear();
    m_nameToId.clear();
    m_adjacency.clear();
    m_maxAuthorsPerPaper = 0;
}

void GraphManager::addAuthorsFromArticle(const std::vector<std::string>& authors) {
    if (authors.size() < 2) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<int> ids;
    ids.reserve(authors.size());
    
    for (const auto& author : authors) {
        auto it = m_nameToId.find(author);
        if (it != m_nameToId.end()) {
            ids.push_back(it->second);
        } else {
            int id = m_authors.size();
            m_authors.push_back(author);
            m_nameToId[author] = id;
            m_adjacency.emplace_back();
            ids.push_back(id);
        }
    }
    
    for (size_t i = 0; i < ids.size(); ++i) {
        for (size_t j = i + 1; j < ids.size(); ++j) {
            m_adjacency[ids[i]].insert(ids[j]);
            m_adjacency[ids[j]].insert(ids[i]);
        }
    }
    
    if ((int)authors.size() > m_maxAuthorsPerPaper) {
        m_maxAuthorsPerPaper = authors.size();
    }
}

int GraphManager::getNodeCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_authors.size();
}

int GraphManager::getEdgeCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    int count = 0;
    for (const auto& neighbors : m_adjacency) {
        count += neighbors.size();
    }
    return count / 2;
}

std::string GraphManager::getAuthorName(int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id >= 0 && id < (int)m_authors.size()) {
        return m_authors[id];
    }
    return "";
}

std::vector<int> GraphManager::getNeighbors(int u) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (u >= 0 && u < (int)m_adjacency.size()) {
        return std::vector<int>(m_adjacency[u].begin(), m_adjacency[u].end());
    }
    return {};
}

std::vector<std::vector<int>> GraphManager::getComponents() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::vector<int>> components;
    std::vector<bool> visited(m_authors.size(), false);
    
    for (int i = 0; i < (int)m_authors.size(); ++i) {
        if (!visited[i]) {
            std::vector<int> comp;
            std::queue<int> q;
            q.push(i);
            visited[i] = true;
            
            while (!q.empty()) {
                int u = q.front();
                q.pop();
                comp.push_back(u);
                for (int v : m_adjacency[u]) {
                    if (!visited[v]) {
                        q.push(v);
                        visited[v] = true;
                    }
                }
            }
            components.push_back(comp);
        }
    }
    
    return components;
}

void GraphManager::addPaperAuthorCount(int count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (count > m_maxAuthorsPerPaper) {
        m_maxAuthorsPerPaper = count;
    }
}

int GraphManager::getMaxAuthorsPerPaper() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxAuthorsPerPaper;
}

bool GraphManager::hasSavedData(const std::string& path) const {
    std::ifstream authorsFile(path + "_authors.dat");
    return authorsFile.is_open();
}

void GraphManager::saveToFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ofstream authorsFile(path + "_authors.dat");
    for (int i = 0; i < (int)m_authors.size(); ++i) {
        authorsFile << i << "|" << m_authors[i] << "\n";
    }
    authorsFile.close();
    
    std::ofstream edgesFile(path + "_edges.dat");
    for (int i = 0; i < (int)m_adjacency.size(); ++i) {
        edgesFile << i;
        for (int neighbor : m_adjacency[i]) {
            edgesFile << " " << neighbor;
        }
        edgesFile << "\n";
    }
    edgesFile.close();
    
    std::ofstream metaFile(path + "_meta.dat");
    metaFile << "节点数: " << m_authors.size() << "\n";
    metaFile << "边数: " << (int)(getEdgeCountInternal()) << "\n";
    metaFile << "最大作者数: " << m_maxAuthorsPerPaper << "\n";
    metaFile.close();
}

int GraphManager::getEdgeCountInternal() const {
    int count = 0;
    for (const auto& neighbors : m_adjacency) {
        count += neighbors.size();
    }
    return count / 2;
}

bool GraphManager::loadFromFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    internalClear();
    
    std::string authorsPath = path + "_authors.dat";

    std::ifstream authorsFile(authorsPath, std::ios::in);
    
    if (!authorsFile.is_open()) {
        return false;
    }
    
    std::string line;
    int authorCount = 0;
    while (std::getline(authorsFile, line)) {
        size_t pos = line.find('|');
        if (pos != std::string::npos) {
            std::string name = line.substr(pos + 1);
            m_authors.push_back(name);
            m_nameToId[name] = m_authors.size() - 1;
            authorCount++;
        }
    }
    authorsFile.close();
     
    m_adjacency.resize(m_authors.size());
    
    std::string edgesPath = path + "_edges.dat";

    std::ifstream edgesFile(edgesPath);
    if (!edgesFile.is_open()) {
        clear();
        return false;
    }
    
    int edgeCount = 0;
    while (std::getline(edgesFile, line)) {
        std::istringstream iss(line);
        int id, neighbor;
        iss >> id;
        while (iss >> neighbor) {
            if (id >= 0 && id < (int)m_adjacency.size()) {
                m_adjacency[id].insert(neighbor);
                edgeCount++;
            }
        }
    }
    edgesFile.close();
    
    std::string metaPath = path + "_meta.dat";
    std::ifstream metaFile(metaPath);
    if (metaFile.is_open()) {
        while (std::getline(metaFile, line)) {
            if (line.find("最大作者数") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    m_maxAuthorsPerPaper = std::stoi(line.substr(pos + 2));
                }
            }
        }
        metaFile.close();
    } else {
        // qDebug() << "DEBUG: meta file not found (optional)";
    }
    return true;
}