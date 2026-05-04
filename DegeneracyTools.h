#ifndef _DJS_DEGENERACY_HELPER_H_
#define _DJS_DEGENERACY_HELPER_H_

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "Tools.h"
#include <list>
#include "MemoryManager.h"

struct NeighborList
{
    NeighborList()
    : vertex(-1)
    , earlier()
    , later()
    , orderNumber(-1) {}

    int vertex; //顶点编号
    std::list<int> earlier; //排在顶点前面的邻居
    std::list<int> later; //排在顶点后面的邻居
    int orderNumber; //顶点的位置
};

typedef struct NeighborList NeighborList;

class NeighborListArray
{
public:
    NeighborListArray()
    : vertex(-1)
    , earlier()
    , earlierDegree(-1)
    , later()
    , laterDegree(-1)
    , orderNumber(-1) {}

    int vertex;
    std::vector<int> earlier;
    int earlierDegree; //前面的邻居数量
    std::vector<int> later;
    int laterDegree; //后面的邻居数量
    int orderNumber;
};

typedef struct NeighborListArray NeighborListArray;

int computeDegeneracy(std::vector<std::list<int>> const &adjList, int size);
int computeDegeneracy(std::vector<std::vector<int>> const &adjList, int size);

NeighborList** computeDegeneracyOrderList(std::vector<std::list<int>> const &adjList, int size);

NeighborListArray** computeDegeneracyOrderArray(std::vector<std::list<int>> const &adjList, int size);

std::vector<NeighborListArray> computeDegeneracyOrderArray(std::vector<std::vector<int>> &adjArray, int size);
std::vector<NeighborListArray> computeDegeneracyOrderArrayWithArrays(std::vector<std::vector<int>> &adjArray, int size);

std::vector<NeighborListArray> computeDegeneracyOrderArrayForReverse(std::vector<std::vector<int>> &adjArray, int size);

int neighborListComparator(void* nl1, void* nl2);

std::vector<NeighborListArray> computeMaximumLaterOrderArray(std::vector<std::vector<int>> &adjArray, int size);

std::vector<int> GetVerticesInDegeneracyOrder(std::vector<std::vector<int>> &adjArray);

#endif
