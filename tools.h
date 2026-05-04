#ifndef _DJS_MISC_H_
#define _DJS_MISC_H_
#include <list>
#include <vector>
#include <string>
#include <stdio.h>
#include <ctime>

class Algorithm;
#include <cmath>

int nodeComparator(int node1, int node2);

void printArray(int* array, int size);

void printArrayWithIndexArrows(int* array, int size, int index1, int index2, int index3);

void printArrayOfLinkedLists(std::vector<std::list<int>> const &listOfLists, int size);

void destroyCliqueResults(std::list<std::list<int>> &cliques);

std::vector<std::list<int>> readInGraphAdjList(int* n, int* m);

std::vector<std::list<int>> readInGraphAdjList(int &n, int &m, std::string const &fileName);
std::vector<std::list<int>> readInGraphAdjListEdgesPerLine(int &n, int &m, std::string const &fileName);

void runAndPrintStatsMatrix(long (*function)(char**,
                                             int),
                            const char* algName,
                            char** adjMatrix,
                            int n );

void RunAndPrintStats(Algorithm* pAlgorithm, std::list<std::list<int>> &cliques, bool const outputLatex);

void printListAbbv(std::list<int> const &linkedList, void (*printFunc)(int));

inline void processClique(std::list<int> const &)
{
#ifdef PRINT_CLIQUES_TOMITA_STYLE
    printf("c ");
#endif
}

void DescribeVertex(int const lineNumber, int *vertexSets, int *vertexLookup, int const size, int const vertex, int const beginX, int const beginD, int const beginP, int const beginR);

void DescribeSet(std::string const &setName, int const begin, int const end);

void DescribeState(int const lineNumber, int *vertexSets, int *vertexLookup, int const size, int const beginX, int const beginD, int const beginP, int const beginR);

void CheckConsistency(int const lineNumber, size_t const recursionNumber, int *vertexSets, int *vertexLookup, int const size);

void CheckReverseConsistency(int const lineNumber, size_t const recursionNumber, int *vertexSets, int *vertexLookup, int const size);

bool IsMaximalClique(std::list<int> const &clique, std::vector<std::vector<int>> const &adjacencyList);

namespace Tools
{
void printList(std::list<int> const &linkedList, void (*printFunc)(int));
void printInt(int integer);
std::vector<int> ReadMetisOrdering(std::string const &filename);
std::string GetTimeInSeconds(clock_t delta, bool brackets=true);
};

#endif
