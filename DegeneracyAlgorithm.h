#ifndef _DJS_DEGENERACY_ALGORITHM_H_
#define _DJS_DEGENERACY_ALGORITHM_H_
#include "Algorithm.h"
#include "Tools.h"
#include "MemoryManager.h"
#include "DegeneracyTools.h"
#include <list>
#include <vector>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class DegeneracyAlgorithm : public Algorithm
{
public:
    DegeneracyAlgorithm(std::vector<std::list<int>> const &adjacencyList);
    virtual ~DegeneracyAlgorithm();

    virtual long Run(std::list<std::list<int>> &cliques);

    DegeneracyAlgorithm           (DegeneracyAlgorithm const &) = delete;
    DegeneracyAlgorithm& operator=(DegeneracyAlgorithm const &) = delete;

    void listAllMaximalCliquesDegeneracyRecursive(long* cliqueCount,
                                                  std::list<int> &partialClique,
                                                  int* vertexSets, int* vertexLookup,
                                                  int** neighborsInP, int* numNeighbors,
                                                  int beginX, int beginP, int beginR);

    long listAllMaximalCliquesDegeneracy(std::vector<std::list<int>> const &adjList, int size);
    
    // 获取找到的最大团
    std::list<int> getMaxClique() const { return m_maxClique; }

private:
    std::vector<std::list<int>> const &m_AdjacencyList;
    
    // 记录最大团
    std::list<int> m_maxClique;
    
    // 存储找到的所有极大团
    std::list<std::list<int>>* m_cliques;
};

#endif
