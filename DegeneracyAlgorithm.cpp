#include "DegeneracyAlgorithm.h"
#include <limits.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "Tools.h"
#include <list>
#include <vector>
#include "MemoryManager.h"
#include "DegeneracyTools.h"

using namespace std;

DegeneracyAlgorithm::DegeneracyAlgorithm(vector<list<int>> const &adjacencyList)
    : Algorithm("degeneracy")
    , m_AdjacencyList(adjacencyList)
{
}

DegeneracyAlgorithm::~DegeneracyAlgorithm()
{
}

long DegeneracyAlgorithm::Run(list<list<int>> &cliques)
{
    m_cliques = &cliques;
    return listAllMaximalCliquesDegeneracy(m_AdjacencyList, m_AdjacencyList.size());
}

inline int findBestPivotNonNeighborsDegeneracy( int** pivotNonNeighbors, int* numNonNeighbors,
                                               int* vertexSets, int* vertexLookup,
                                               int** neighborsInP, int* numNeighbors,
                                               int beginX, int beginP, int beginR)
{
    int pivot = -1;
    int maxIntersectionSize = -1;

    int j = beginX;
    while(j<beginR)
    {
        int vertex = vertexSets[j];
        int numPotentialNeighbors = min(beginR - beginP, numNeighbors[vertex]);

        int numNeighborsInP = 0;

        int k = 0;
        while(k<numPotentialNeighbors)
        {
            int neighbor = neighborsInP[vertex][k];
            int neighborLocation = vertexLookup[neighbor];

            if(neighborLocation >= beginP && neighborLocation < beginR)
            {
                numNeighborsInP++;
            }
            else
            {
                break;
            }

            k++;
        }

        if(numNeighborsInP > maxIntersectionSize)
        {
            pivot = vertex;
            maxIntersectionSize = numNeighborsInP;
        }

        j++;
    }

    *pivotNonNeighbors = (int*)Calloc(beginR-beginP, sizeof(int));
    memcpy(*pivotNonNeighbors, &vertexSets[beginP], (beginR-beginP)*sizeof(int));

    *numNonNeighbors = beginR-beginP;

    int numPivotNeighbors = min(beginR - beginP, numNeighbors[pivot]);

    j = 0;
    while(j<numPivotNeighbors)
    {
        int neighbor = neighborsInP[pivot][j];
        int neighborLocation = vertexLookup[neighbor];

        if(neighborLocation >= beginP && neighborLocation < beginR)
        {
            (*pivotNonNeighbors)[neighborLocation-beginP] = -1;
        }
        else
        {
            break;
        }

        j++;
    }

    j = 0;
    while(j<*numNonNeighbors)
    {
        int vertex = (*pivotNonNeighbors)[j];

        if(vertex == -1)
        {
            (*numNonNeighbors)--;
            (*pivotNonNeighbors)[j] = (*pivotNonNeighbors)[*numNonNeighbors];
            continue;
        }

        j++;
    }

    return pivot;
}

inline void fillInPandXForRecursiveCallDegeneracy( int vertex, int orderNumber,
                                                  int* vertexSets, int* vertexLookup,
                                                  NeighborListArray** orderingArray,
                                                  int** neighborsInP, int* numNeighbors,
                                                  int* pBeginX, int *pBeginP, int *pBeginR,
                                                  int* pNewBeginX, int* pNewBeginP, int *pNewBeginR)
{
    (void)pBeginX;
    (void)pBeginP;
    int vertexLocation = vertexLookup[vertex];

    (*pBeginR)--;
    vertexSets[vertexLocation] = vertexSets[*pBeginR];
    vertexLookup[vertexSets[*pBeginR]] = vertexLocation;
    vertexSets[*pBeginR] = vertex;
    vertexLookup[vertex] = *pBeginR;

    *pNewBeginR = *pBeginR;
    *pNewBeginP = *pBeginR;

    int j = 0;
    while(j<orderingArray[orderNumber]->laterDegree)
    {
        int neighbor = orderingArray[orderNumber]->later[j];
        int neighborLocation = vertexLookup[neighbor];

        (*pNewBeginP)--;

        vertexSets[neighborLocation] = vertexSets[*pNewBeginP];
        vertexLookup[vertexSets[*pNewBeginP]] = neighborLocation;
        vertexSets[*pNewBeginP] = neighbor;
        vertexLookup[neighbor] = *pNewBeginP;

        j++;
    }

    *pNewBeginX = *pNewBeginP;

    j = 0;
    while(j<orderingArray[orderNumber]->earlierDegree)
    {
        int neighbor = orderingArray[orderNumber]->earlier[j];
        int neighborLocation = vertexLookup[neighbor];

        (*pNewBeginX)--;
        vertexSets[neighborLocation] = vertexSets[*pNewBeginX];
        vertexLookup[vertexSets[*pNewBeginX]] = neighborLocation;
        vertexSets[*pNewBeginX] = neighbor;
        vertexLookup[neighbor] = *pNewBeginX;

        Free(neighborsInP[neighbor]);
        neighborsInP[neighbor] = (int*)Calloc(min(*pNewBeginR-*pNewBeginP,orderingArray[neighbor]->laterDegree), sizeof(int));
        numNeighbors[neighbor] = 0;

        int k = 0;
        while(k<orderingArray[neighbor]->laterDegree)
        {
            int laterNeighbor = orderingArray[neighbor]->later[k];
            int laterNeighborLocation = vertexLookup[laterNeighbor];
            if(laterNeighborLocation >= *pNewBeginP && laterNeighborLocation < *pNewBeginR)
            {
                neighborsInP[neighbor][numNeighbors[neighbor]] = laterNeighbor;
                numNeighbors[neighbor]++;
            }

            k++;
        }

        j++;

    }

    j = *pNewBeginP;
    while(j<*pNewBeginR)
    {
        int vertexInP = vertexSets[j];
        numNeighbors[vertexInP] = 0;
        Free(neighborsInP[vertexInP]);
        neighborsInP[vertexInP]=(int*)Calloc( min( *pNewBeginR-*pNewBeginP,
                                                     orderingArray[vertexInP]->laterDegree
                                                         + orderingArray[vertexInP]->earlierDegree), sizeof(int));

        j++;
    }

    j = *pNewBeginP;
    while(j<*pNewBeginR)
    {
        int vertexInP = vertexSets[j];

        int k = 0;
        while(k<orderingArray[vertexInP]->laterDegree)
        {
            int laterNeighbor = orderingArray[vertexInP]->later[k];
            int laterNeighborLocation = vertexLookup[laterNeighbor];

            if(laterNeighborLocation >= *pNewBeginP && laterNeighborLocation < *pNewBeginR)
            {
                neighborsInP[vertexInP][numNeighbors[vertexInP]] = laterNeighbor;
                numNeighbors[vertexInP]++;
                neighborsInP[laterNeighbor][numNeighbors[laterNeighbor]] = vertexInP;
                numNeighbors[laterNeighbor]++;
            }

            k++;
        }

        j++;
    }
}

static unsigned long largestDifference(0);
static unsigned long numLargeJumps;
static unsigned long stepsSinceLastReportedClique(0);

long DegeneracyAlgorithm::listAllMaximalCliquesDegeneracy(vector<list<int>> const &adjList, int size)
{
    int* vertexSets = (int*)Calloc(size, sizeof(int));

    int* vertexLookup = (int*)Calloc(size, sizeof(int));

    int** neighborsInP = (int**)Calloc(size, sizeof(int*));
    int* numNeighbors = (int*)Calloc(size, sizeof(int));

    NeighborListArray** orderingArray = computeDegeneracyOrderArray(adjList, size);

    int i = 0;

    while(i<size)
    {
        vertexLookup[i] = i;
        vertexSets[i] = i;
        neighborsInP[i] = (int*)Calloc(1, sizeof(int));
        numNeighbors[i] = 1;
        i++;
    }

    int beginX = 0;
    int beginP = 0;
    int beginR = size;

    long cliqueCount = 0;

    list<int> partialClique;

    for(i=0;i<size;i++)
    {
        int vertex = (int)orderingArray[i]->vertex;

#ifdef PRINT_CLIQUES_TOMITA_STYLE
        printf("%d ", vertex);
#endif

        partialClique.push_back(vertex);

        int newBeginX, newBeginP, newBeginR;

        fillInPandXForRecursiveCallDegeneracy( i, vertex,
                                              vertexSets, vertexLookup,
                                              orderingArray,
                                              neighborsInP, numNeighbors,
                                              &beginX, &beginP, &beginR,
                                              &newBeginX, &newBeginP, &newBeginR);

        listAllMaximalCliquesDegeneracyRecursive(&cliqueCount,
                                                 partialClique,
                                                 vertexSets, vertexLookup,
                                                 neighborsInP, numNeighbors,
                                                 newBeginX, newBeginP, newBeginR);

#ifdef PRINT_CLIQUES_TOMITA_STYLE
        printf("b ");
#endif

        beginR = beginR + 1;

        partialClique.pop_back();
    }

    partialClique.clear();

    Free(vertexSets);
    Free(vertexLookup);

    for(i = 0; i<size; i++)
    {
        Free(neighborsInP[i]);
        delete orderingArray[i];
    }

    Free(orderingArray);
    Free(neighborsInP);
    Free(numNeighbors);

    return cliqueCount;
}

inline void moveToRDegeneracy( int vertex,
                              int* vertexSets, int* vertexLookup,
                              int** neighborsInP, int* numNeighbors,
                              int* pBeginX, int *pBeginP, int *pBeginR,
                              int* pNewBeginX, int* pNewBeginP, int *pNewBeginR)
{

    int vertexLocation = vertexLookup[vertex];

    (*pBeginR)--;
    vertexSets[vertexLocation] = vertexSets[*pBeginR];
    vertexLookup[vertexSets[*pBeginR]] = vertexLocation;
    vertexSets[*pBeginR] = vertex;
    vertexLookup[vertex] = *pBeginR;

    *pNewBeginX = *pBeginP;
    *pNewBeginP = *pBeginP;
    *pNewBeginR = *pBeginP;

    int sizeOfP = *pBeginR - *pBeginP;

    int j = *pBeginX;
    while(j<*pNewBeginX)
    {
        int neighbor = vertexSets[j];
        int neighborLocation = j;

        int incrementJ = 1;

        int numPotentialNeighbors = min(sizeOfP, numNeighbors[neighbor]);

        int k = 0;
        while(k<numPotentialNeighbors)
        {
            if(neighborsInP[neighbor][k] == vertex)
            {
                (*pNewBeginX)--;
                vertexSets[neighborLocation] = vertexSets[(*pNewBeginX)];
                vertexLookup[vertexSets[(*pNewBeginX)]] = neighborLocation;
                vertexSets[(*pNewBeginX)] = neighbor;
                vertexLookup[neighbor] = (*pNewBeginX);
                incrementJ=0;
            }

            k++;
        }

        if(incrementJ) j++;
    }

    j = (*pBeginP);
    while(j<(*pBeginR))
    {
        int neighbor = vertexSets[j];
        int neighborLocation = j;

        int numPotentialNeighbors = min(sizeOfP, numNeighbors[neighbor]);

        int k = 0;
        while(k<numPotentialNeighbors)
        {
            if(neighborsInP[neighbor][k] == vertex)
            {
                vertexSets[neighborLocation] = vertexSets[(*pNewBeginR)];
                vertexLookup[vertexSets[(*pNewBeginR)]] = neighborLocation;
                vertexSets[(*pNewBeginR)] = neighbor;
                vertexLookup[neighbor] = (*pNewBeginR);
                (*pNewBeginR)++;
            }

            k++;
        }

        j++;
    }

    j = (*pNewBeginX);

    while(j < *pNewBeginR)
    {
        int thisVertex = vertexSets[j];

        int numPotentialNeighbors = min(sizeOfP, numNeighbors[thisVertex]);

        int numNeighborsInP = 0;

        int k = 0;
        while(k < numPotentialNeighbors)
        {
            int neighbor = neighborsInP[thisVertex][k];
            int neighborLocation = vertexLookup[neighbor];
            if(neighborLocation >= *pNewBeginP && neighborLocation < *pNewBeginR)
            {
                neighborsInP[thisVertex][k] = neighborsInP[thisVertex][numNeighborsInP];
                neighborsInP[thisVertex][numNeighborsInP] = neighbor;
                numNeighborsInP++;
            }
            k++;
        }

        j++;
    }
}

inline void moveFromRToXDegeneracy( int vertex,
                                   int* vertexSets, int* vertexLookup,
                                   int* pBeginX, int* pBeginP, int* pBeginR )
{
    (void)pBeginX;
    int vertexLocation = vertexLookup[vertex];

    vertexSets[vertexLocation] = vertexSets[*pBeginP];
    vertexLookup[vertexSets[*pBeginP]] = vertexLocation;
    vertexSets[*pBeginP] = vertex;
    vertexLookup[vertex] = *pBeginP;

    *pBeginP = *pBeginP + 1;
    *pBeginR = *pBeginR + 1;
}

void DegeneracyAlgorithm::listAllMaximalCliquesDegeneracyRecursive(long* cliqueCount,
                                                                   list<int> &partialClique,
                                                                   int* vertexSets, int* vertexLookup,
                                                                   int** neighborsInP, int* numNeighbors,
                                                                   int beginX, int beginP, int beginR)
{

    stepsSinceLastReportedClique++;

    if(beginX >= beginP && beginP >= beginR)
    {
        (*cliqueCount)++;

        if (stepsSinceLastReportedClique > partialClique.size()) {
            numLargeJumps++;
            if (largestDifference < (stepsSinceLastReportedClique - partialClique.size())) {
                largestDifference = stepsSinceLastReportedClique - partialClique.size();
            }
        }

        stepsSinceLastReportedClique = 0;

        ExecuteCallBacks(partialClique);
        processClique(partialClique);
        
        // 记录最大团
        if (partialClique.size() > m_maxClique.size()) {
            m_maxClique = partialClique;
        }
        
        // 添加到结果列表
        if (m_cliques) {
            m_cliques->push_back(partialClique);
        }

        return;
    }

    if(beginP >= beginR)
        return;

    int* myCandidatesToIterateThrough;
    int numCandidatesToIterateThrough;

    findBestPivotNonNeighborsDegeneracy( &myCandidatesToIterateThrough,
                                        &numCandidatesToIterateThrough,
                                        vertexSets, vertexLookup,
                                        neighborsInP, numNeighbors,
                                        beginX, beginP, beginR);

    if(numCandidatesToIterateThrough != 0)
    {
        int iterator = 0;
        while(iterator < numCandidatesToIterateThrough)
        {
            int vertex = myCandidatesToIterateThrough[iterator];

#ifdef PRINT_CLIQUES_TOMITA_STYLE
            printf("%d ", vertex);
#endif

            int newBeginX, newBeginP, newBeginR;

            partialClique.push_back(vertex);

            moveToRDegeneracy( vertex,
                              vertexSets, vertexLookup,
                              neighborsInP, numNeighbors,
                              &beginX, &beginP, &beginR,
                              &newBeginX, &newBeginP, &newBeginR);

            listAllMaximalCliquesDegeneracyRecursive(cliqueCount,
                                                     partialClique,
                                                     vertexSets, vertexLookup,
                                                     neighborsInP, numNeighbors,
                                                     newBeginX, newBeginP, newBeginR);

#ifdef PRINT_CLIQUES_TOMITA_STYLE
            printf("b ");
#endif

            partialClique.pop_back();

            moveFromRToXDegeneracy( vertex,
                                   vertexSets, vertexLookup,
                                   &beginX, &beginP, &beginR );

            iterator++;
        }

        iterator = 0;

        while(iterator < numCandidatesToIterateThrough)
        {
            int vertex = myCandidatesToIterateThrough[iterator];
            int vertexLocation = vertexLookup[vertex];

            beginP--;
            vertexSets[vertexLocation] = vertexSets[beginP];
            vertexSets[beginP] = vertex;
            vertexLookup[vertex] = beginP;
            vertexLookup[vertexSets[vertexLocation]] = vertexLocation;

            iterator++;
        }
    }

    Free(myCandidatesToIterateThrough);
}
