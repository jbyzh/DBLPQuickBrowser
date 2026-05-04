#include "collaborationgraph.h"
#include <thread>
#include <atomic>
#include <cmath>
#include <algorithm>
#include "DegeneracyAlgorithm.h"

CollaborationGraph::CollaborationGraph() : current_id(0) {
}

CollaborationGraph::~CollaborationGraph() {
}

int CollaborationGraph::addAuthor(const std::string& name) {
    if (name_to_id.find(name) != name_to_id.end()) {
        return name_to_id[name]; // 已存在，返回编号
    }

    auto node = std::make_shared<AuthorNode>(name, current_id);
    nodes.push_back(node);
    name_to_id[name] = current_id;
    return current_id++;
}

void CollaborationGraph::addEdge(const std::string& name1, const std::string& name2) {
    if (name1 == name2) return;

    int id1 = addAuthor(name1);
    int id2 = addAuthor(name2);

    nodes[id1]->neighbors.insert(id2);
    nodes[id2]->neighbors.insert(id1);
}

std::set<int> CollaborationGraph::getNeighbors(int u) const {
    if (u < 0 || u >= static_cast<int>(nodes.size())) {
        return std::set<int>();
    }
    return std::set<int>(nodes[u]->neighbors.begin(), nodes[u]->neighbors.end());
}

std::vector<std::vector<int>> CollaborationGraph::getComponents() {
    std::vector<std::vector<int>> components;
    std::vector<bool> visited(nodes.size(), false);

    for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
        if (!visited[i]) {
            std::vector<int> comp;
            std::queue<int> q;
            q.push(i);
            visited[i] = true;

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                comp.push_back(u);
                for (auto& v : nodes[u]->neighbors) {
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

std::vector<std::vector<int>> CollaborationGraph::findMaximalCliquesBronKerbosch(const std::vector<int>& component) {
    std::vector<std::vector<int>> result;
    
    if (component.empty()) {
        return result;
    }
    
    // 需要把 component 的节点转换为 0-based 的 adjacency list 给 DegeneracyAlgorithm
    std::unordered_map<int, int> originalToNew;
    std::unordered_map<int, int> newToOriginal;
    
    for (size_t i = 0; i < component.size(); ++i) {
        originalToNew[component[i]] = i;
        newToOriginal[i] = component[i];
    }
    
    std::vector<std::list<int>> adjacencyList(component.size());
    
    for (size_t i = 0; i < component.size(); ++i) {
        int u = component[i];
        for (int v : nodes[u]->neighbors) {
            auto it = originalToNew.find(v);
            if (it != originalToNew.end()) {
                adjacencyList[i].push_back(it->second);
            }
        }
    }
    
    DegeneracyAlgorithm algo(adjacencyList);
    std::list<std::list<int>> cliques;
    algo.Run(cliques);
    
    for (const auto& clique : cliques) {
        std::vector<int> newClique;
        for (int node : clique) {
            newClique.push_back(newToOriginal[node]);
        }
        result.push_back(newClique);
    }
    
    return result;
}

int CollaborationGraph::getNodeCount() const {
    return nodes.size();
}

int CollaborationGraph::getEdgeCount() const {
    int edgeCount = 0;
    for (const auto& node : nodes) {
        edgeCount += node->neighbors.size();
    }
    // 每条边被存储了两次，所以除以2
    return edgeCount / 2;
}

std::string CollaborationGraph::getAuthorName(int id) const {
    if (id < 0 || id >= static_cast<int>(nodes.size())) {
        return "";
    }
    return nodes[id]->name;
}
