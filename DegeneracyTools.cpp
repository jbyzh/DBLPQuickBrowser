#include "Tools.h"
#include "MemoryManager.h"
#include "DegeneracyTools.h"
#include <climits>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

int computeDegeneracy(vector<list<int>> const &adjList, int size)
{
    int i = 0;

    int degeneracy = 0;

    vector<list<int>> verticesByDegree(size);

    vector<std::list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjList[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    while(numVerticesRemoved < size)
    {
        if(!verticesByDegree[currentDegree].empty())
        {
            degeneracy = max(degeneracy,currentDegree);

            int const vertex = verticesByDegree[currentDegree].front();

            verticesByDegree[currentDegree].erase(vertexLocator[vertex]);

            degree[vertex] = -1;

            list<int> const &neighborList = adjList[vertex];

            for(int const neighbor : neighborList)
            {
                if(degree[neighbor]!=-1)
                {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    degree[neighbor]--;

                    if(degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else
        {
            currentDegree++;
        }
    }

    verticesByDegree.clear();

    return degeneracy;
}

int computeDegeneracy(vector<vector<int>> const &adjList, int size)
{
    int i = 0;

    int degeneracy = 0;

    vector<list<int>> verticesByDegree(size);

    vector<std::list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjList[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    while(numVerticesRemoved < size)
    {
        if(!verticesByDegree[currentDegree].empty())
        {
            degeneracy = max(degeneracy,currentDegree);

            int const vertex = verticesByDegree[currentDegree].front();

            verticesByDegree[currentDegree].erase(vertexLocator[vertex]);

            degree[vertex] = -1;

            vector<int> const &neighborList = adjList[vertex];

            for(int const neighbor : neighborList)
            {
                if(degree[neighbor]!=-1)
                {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    degree[neighbor]--;

                    if(degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else
        {
            currentDegree++;
        }
    }

    verticesByDegree.clear();

    return degeneracy;
}


NeighborList** computeDegeneracyOrderList(vector<list<int>> const &adjList, int size)
{

#ifdef DEBUG
    printf("degeneracy is %d\n", computeDegeneracy(list, size));
#endif

    NeighborList** ordering = (NeighborList**)Calloc(size, sizeof(NeighborList*));

    int i = 0;

    int degeneracy = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i = 0; i < size; i++)
    {
        ordering[i] = (NeighborList*)Malloc(sizeof(NeighborList));
    }

    for(i=0; i<size; i++)
    {
        degree[i] = adjList[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    while(numVerticesRemoved < size)
    {
        if(!verticesByDegree[currentDegree].empty())
        {
            degeneracy = max(degeneracy,currentDegree);

            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].erase(vertexLocator[vertex]);

            ordering[vertex]->vertex = vertex;
            ordering[vertex]->orderNumber = numVerticesRemoved;

            degree[vertex] = -1;

            list<int> const &neighborList = adjList[vertex];

            for(int const neighbor : neighborList)
            {
                if(degree[neighbor]!=-1)
                {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);
                    ordering[vertex]->later.push_back(neighbor);

                    degree[neighbor]--;

                    if(degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
                else
                {
                    ordering[vertex]->earlier.push_back(neighbor);
                }
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else
        {
            currentDegree++;
        }
    }

    return ordering;
}

NeighborListArray** computeDegeneracyOrderArray(vector<list<int>> const &adjList, int size)
{

    vector<NeighborList> vOrdering(size);

    int i = 0;

    int degeneracy = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjList[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    while(numVerticesRemoved < size)
    {
        if(!verticesByDegree[currentDegree].empty())
        {
            degeneracy = max(degeneracy,currentDegree);

            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vOrdering[vertex].vertex = vertex;
            vOrdering[vertex].orderNumber = numVerticesRemoved;

            degree[vertex] = -1;

            list<int> const &neighborList = adjList[vertex];

            for(int const neighbor : neighborList)
            {
                if(degree[neighbor]!=-1)
                {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);
                    (vOrdering[vertex].later).push_back(neighbor);

                    degree[neighbor]--;

                    if(degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
                else
                {
                    vOrdering[vertex].earlier.push_back(neighbor);
                }
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else
        {
            currentDegree++;
        }

    }

    NeighborListArray** orderingArray = (NeighborListArray**)Calloc(size, sizeof(NeighborListArray*));

    for(i = 0; i<size;i++)
    {
        orderingArray[i] = new NeighborListArray();
        orderingArray[i]->vertex = vOrdering[i].vertex;
        orderingArray[i]->orderNumber = vOrdering[i].orderNumber;

        orderingArray[i]->laterDegree = vOrdering[i].later.size();
        orderingArray[i]->later.resize(orderingArray[i]->laterDegree);

        int j=0;
        for(int const laterNeighbor : vOrdering[i].later)
        {
            orderingArray[i]->later[j++] = laterNeighbor;
        }

        orderingArray[i]->earlierDegree = vOrdering[i].earlier.size();
        orderingArray[i]->earlier.resize(orderingArray[i]->earlierDegree);

        j=0;
        for (int const earlierNeighbor : vOrdering[i].earlier)
        {
            orderingArray[i]->earlier[j++] = earlierNeighbor;
        }
    }

    return orderingArray;
}

vector<NeighborListArray> computeMaximumLaterOrderArray(vector<vector<int>> &adjArray, int size)
{
    int i = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjArray[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = size-1;

    int numVerticesRemoved = 0;

    vector<NeighborListArray> vOrderingArray(size);

    while (numVerticesRemoved < size) {
        if (!verticesByDegree[currentDegree].empty()) {
            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vOrderingArray[vertex].vertex = vertex;
            vOrderingArray[vertex].orderNumber = numVerticesRemoved;

            degree[vertex] = -1;

            vector<int> &neighborList = adjArray[vertex];

            int splitPoint(neighborList.size());
            for(int i=0; i < splitPoint; ++i) {
                int const neighbor(neighborList[i]);
                if(degree[neighbor]!=-1) {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    neighborList[i] = neighborList[--splitPoint];
                    neighborList[splitPoint] = neighbor;
                    i--;

                    degree[neighbor]--;

                    if (degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            vOrderingArray[vertex].laterDegree = neighborList.size() - splitPoint;
            vOrderingArray[vertex].later.resize(neighborList.size() - splitPoint);

            vOrderingArray[vertex].earlierDegree = splitPoint;
            vOrderingArray[vertex].earlier.resize(splitPoint);

            for (int i = 0; i < splitPoint; ++i) {
                vOrderingArray[vertex].earlier[i] = neighborList[i];
            }

            for (int i = splitPoint; i < static_cast<int>(neighborList.size()); ++i) {
                vOrderingArray[vertex].later[i-splitPoint] = neighborList[i];
            }

            numVerticesRemoved++;
        }
        else {
            currentDegree--;
        }
    }

    return vOrderingArray;
}

vector<NeighborListArray> computeDegeneracyOrderArray(vector<vector<int>> &adjArray, int size)
{
    int i = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjArray[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    vector<NeighborListArray> vOrderingArray(size);

    while (numVerticesRemoved < size) {
        if (!verticesByDegree[currentDegree].empty()) {
            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vOrderingArray[vertex].vertex = vertex;
            vOrderingArray[vertex].orderNumber = numVerticesRemoved;

            degree[vertex] = -1;

            vector<int> &neighborList = adjArray[vertex];

            int splitPoint(neighborList.size());
            for(int i=0; i < splitPoint; ++i) {
                int const neighbor(neighborList[i]);
                if(degree[neighbor]!=-1) {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    neighborList[i] = neighborList[--splitPoint];
                    neighborList[splitPoint] = neighbor;
                    i--;

                    degree[neighbor]--;

                    if (degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            vOrderingArray[vertex].laterDegree = neighborList.size() - splitPoint;
            vOrderingArray[vertex].later.resize(neighborList.size() - splitPoint);

            vOrderingArray[vertex].earlierDegree = splitPoint;
            vOrderingArray[vertex].earlier.resize(splitPoint);

            for (int i = 0; i < splitPoint; ++i) {
                vOrderingArray[vertex].earlier[i] = neighborList[i];
            }

            for (int i = splitPoint; i < static_cast<int>(neighborList.size()); ++i) {
                vOrderingArray[vertex].later[i-splitPoint] = neighborList[i];
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else {
            currentDegree++;
        }
    }

    return vOrderingArray;
}

vector<NeighborListArray> computeDegeneracyOrderArrayWithArrays(vector<vector<int>> &adjArray, int size)
{
    vector<int> verticesOrderedByDegree(size);

    vector<int> firstIndexWithDegree(size);

    vector<int> vertexLocator(size);

    vector<int> degree(size);
    {
        vector<list<int>> verticesByDegree(size);

        for (int i=0; i<size; i++) {
            degree[i] = adjArray[i].size();
            verticesByDegree[degree[i]].push_back(i);
        }

        int vertexInsertionCount(0);
        for (int i = 0; i < size; ++i) {
            int const startingIndex(vertexInsertionCount);
            for (int const vertex : verticesByDegree[i]) {
                verticesOrderedByDegree[vertexInsertionCount] = vertex;
                vertexLocator[vertex] = vertexInsertionCount++;
            }

            if (startingIndex != vertexInsertionCount) {
                firstIndexWithDegree[i] = startingIndex;
            } else {
                firstIndexWithDegree[i] = -1;
            }
        }
        verticesByDegree.clear();
    }

    vector<NeighborListArray> vOrderingArray(size);

    for (int currentVertexIndex = 0; currentVertexIndex < size; currentVertexIndex++) {
        int const vertex = verticesOrderedByDegree[currentVertexIndex];
        int const currentDegree(degree[vertex]);

        if (currentVertexIndex != size-1) {
            int const nextVertexDegree(degree[verticesOrderedByDegree[currentVertexIndex+1]]);
            if (nextVertexDegree == currentDegree) {
                firstIndexWithDegree[currentDegree] = currentVertexIndex+1;
            } else {
                firstIndexWithDegree[currentDegree] = -1;
            }
        }

        vOrderingArray[vertex].vertex = vertex;
        vOrderingArray[vertex].orderNumber = currentVertexIndex;

        degree[vertex] = -1;

        vector<int> &neighborList = adjArray[vertex];

        int splitPoint(neighborList.size());
        for (int i=0; i < splitPoint; ++i) {
            int const neighbor(neighborList[i]);
            if (degree[neighbor] != -1) {
                int const firstIndexWithSameDegree(firstIndexWithDegree[degree[neighbor]]);

                if (vertexLocator[neighbor] != firstIndexWithSameDegree) {
                    int const indexOfNeighbor(vertexLocator[neighbor]);
                    int const firstVertexWithSameDegree(verticesOrderedByDegree[firstIndexWithSameDegree]);

                    verticesOrderedByDegree[firstIndexWithSameDegree] = neighbor;
                    vertexLocator[neighbor] = firstIndexWithSameDegree;

                    verticesOrderedByDegree[indexOfNeighbor] = firstVertexWithSameDegree;
                    vertexLocator[firstVertexWithSameDegree] = indexOfNeighbor;
                }

                if (firstIndexWithSameDegree == (size - 1) || degree[verticesOrderedByDegree[firstIndexWithSameDegree+1]] != degree[neighbor]) {
                    firstIndexWithDegree[degree[neighbor]] = -1;
                } else {
                    firstIndexWithDegree[degree[neighbor]] = firstIndexWithSameDegree + 1;
                }

                neighborList[i] = neighborList[--splitPoint];
                neighborList[splitPoint] = neighbor;
                i--;

                degree[neighbor]--;

                if (degree[neighbor] != -1 && firstIndexWithDegree[degree[neighbor]] == -1) {
                    firstIndexWithDegree[degree[neighbor]] = vertexLocator[neighbor];
                }
            }
        }

        vOrderingArray[vertex].laterDegree = neighborList.size() - splitPoint;
        vOrderingArray[vertex].later.resize(neighborList.size() - splitPoint);

        vOrderingArray[vertex].earlierDegree = splitPoint;
        vOrderingArray[vertex].earlier.resize(splitPoint);

        for (int i = 0; i < splitPoint; ++i) {
            vOrderingArray[vertex].earlier[i] = neighborList[i];
        }

        for (int i = splitPoint; i < static_cast<int>(neighborList.size()); ++i) {
            vOrderingArray[vertex].later[i-splitPoint] = neighborList[i];
        }
    }

    return vOrderingArray;
}

vector<NeighborListArray> computeDegeneracyOrderArrayForReverse(vector<vector<int>> &adjArray, int size)
{
    int i = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<size; i++)
    {
        degree[i] = adjArray[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    vector<NeighborListArray> vOrderingArray(size);

    while (numVerticesRemoved < size) {
        if (!verticesByDegree[currentDegree].empty()) {
            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vOrderingArray[vertex].vertex = vertex;
            vOrderingArray[vertex].orderNumber = numVerticesRemoved;

            degree[vertex] = -1;

            vector<int> &neighborList = adjArray[vertex];

            int splitPoint(neighborList.size());
            for(int i=0; i < splitPoint; ++i) {
                int const neighbor(neighborList[i]);
                if(degree[neighbor]!=-1) {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    neighborList[i] = neighborList[--splitPoint];
                    neighborList[splitPoint] = neighbor;
                    i--;

                    degree[neighbor]--;

                    if (degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            vOrderingArray[vertex].laterDegree = neighborList.size() - splitPoint;
            vOrderingArray[vertex].later.resize(neighborList.size() - splitPoint);

            vOrderingArray[vertex].earlierDegree = splitPoint;
            vOrderingArray[vertex].earlier.resize(splitPoint);

            for (int i = 0; i < splitPoint; ++i) {
                vOrderingArray[vertex].earlier[i] = neighborList[i];
            }

            for (int i = splitPoint; i < static_cast<int>(neighborList.size()); ++i) {
                vOrderingArray[vertex].later[i-splitPoint] = neighborList[i];
            }

            auto compareOrderNumber = [&vOrderingArray] (int const left, int const right) { return vOrderingArray[left].orderNumber < vOrderingArray[right].orderNumber; };

            sort(vOrderingArray[vertex].earlier.begin(), vOrderingArray[vertex].earlier.end(), compareOrderNumber);

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else {
            currentDegree++;
        }
    }

    return vOrderingArray;
}

vector<int> GetVerticesInDegeneracyOrder(vector<vector<int>> &adjArray)
{
    size_t const size(adjArray.size());
    vector<int> vResult(size, -1);

#if 0
    // array of lists of vertices, indexed by degree
    vector<list<int>> verticesByDegree(size);

    // array of lists of vertices, indexed by degree
    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    // fill each cell of degree lookup table
    // then use that degree to populate the
    // lists of vertices indexed by degree

    for (size_t i = 0; i < size; i++) {
        degree[i] = adjArray[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;
    int numVerticesRemoved = 0;

    while (numVerticesRemoved < size) {
        if (!verticesByDegree[currentDegree].empty()) {
            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vResult[numVerticesRemoved] = vertex;

            for (int const neighbor : adjArray[vertex]) {
                if(degree[neighbor]!=-1) {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    degree[neighbor]--;

                    if (degree[neighbor] != -1) {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            numVerticesRemoved++;
            currentDegree = 0;
        } else {
            currentDegree++;
        }
    }
#else
    int i = 0;

    vector<list<int>> verticesByDegree(size);

    vector<list<int>::iterator> vertexLocator(size);

    vector<int> degree(size);

    for(i=0; i<static_cast<int>(size); i++)
    {
        degree[i] = adjArray[i].size();
        verticesByDegree[degree[i]].push_front(i);
        vertexLocator[i] = verticesByDegree[degree[i]].begin();
    }

    int currentDegree = 0;

    int numVerticesRemoved = 0;

    vector<NeighborListArray> vOrderingArray(size);

    while (numVerticesRemoved < static_cast<int>(size)) {
        if (!verticesByDegree[currentDegree].empty()) {
            int const vertex = verticesByDegree[currentDegree].front();
            verticesByDegree[currentDegree].pop_front();

            vOrderingArray[vertex].vertex = vertex;
            vOrderingArray[vertex].orderNumber = numVerticesRemoved;
            vResult[numVerticesRemoved] = vertex;

            degree[vertex] = -1;

            vector<int> &neighborList = adjArray[vertex];

            int splitPoint(neighborList.size());
            for(int i=0; i < splitPoint; ++i) {
                int const neighbor(neighborList[i]);
                if(degree[neighbor]!=-1) {
                    verticesByDegree[degree[neighbor]].erase(vertexLocator[neighbor]);

                    neighborList[i] = neighborList[--splitPoint];
                    neighborList[splitPoint] = neighbor;
                    i--;

                    degree[neighbor]--;

                    if (degree[neighbor] != -1)
                    {
                        verticesByDegree[degree[neighbor]].push_front(neighbor);
                        vertexLocator[neighbor] = verticesByDegree[degree[neighbor]].begin();
                    }
                }
            }

            vOrderingArray[vertex].laterDegree = neighborList.size() - splitPoint;
            vOrderingArray[vertex].later.resize(neighborList.size() - splitPoint);

            vOrderingArray[vertex].earlierDegree = splitPoint;
            vOrderingArray[vertex].earlier.resize(splitPoint);

            for (int i = 0; i < splitPoint; ++i) {
                vOrderingArray[vertex].earlier[i] = neighborList[i];
            }

            for (int i = splitPoint; i < static_cast<int>(neighborList.size()); ++i) {
                vOrderingArray[vertex].later[i-splitPoint] = neighborList[i];
            }

            numVerticesRemoved++;
            currentDegree = 0;
        }
        else {
            currentDegree++;
        }
    }

#endif

    return vResult;
}
