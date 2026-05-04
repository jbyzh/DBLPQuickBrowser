#include <cassert>
#include <cstdio>
#include <ctime>
#include <fstream>
#include "Tools.h"
#include <list>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "MemoryManager.h"
#include "Algorithm.h"

using namespace std;

int nodeComparator(int node1, int node2)
{
    if (node1 < node2)
        return -1;
    if(node1 > node2)
        return 1;

    return 0;
}

int sortComparator(int node1, int node2)
{
    if (node1 < node2)
        return -1;
    if(node1 > node2)
        return 1;

    return 0;
}

void printArray(int* array, int size)
{
    int i = 0;
    while(i<size)
        printf("%d ", array[i++]);
    printf("\n");
}

void printArrayWithIndexArrows(int* array, int size, int index1, int index2, int index3)
{
    printArray(array, size);
    int i = 0;
    while (i++ < index1)
        printf(" ");
    printf("^");

    while (i++ < index2)
        printf(" ");
    printf("^");

    while (i++ < index3)
        printf(" ");
    printf("^");

    printf("\n");
}

void printArrayOfLinkedLists(vector<list<int>> const &arrayOfLists, int size)
{
    (void)size;
    int i = 0;
    while (i < static_cast<int>(arrayOfLists.size()))
    {
        if (!arrayOfLists[i].empty())
        {
            printf("%d:", i);
            printListAbbv(arrayOfLists[i], &Tools::printInt);
        }
        i++;
    }
}

void printClique(int* clique)
{
    int i = 0;
    while(clique[i]!=-1)
    {
        printf("%d", clique[i]);
        if(clique[i+1]!=-1)
            printf(" ");
        i++;
    }
    printf("\n");
}

void Tools::printInt(int integer)
{
    printf("%d", integer);
}

void destroyCliqueResults(list<list<int>> &cliques)
{
    cliques.clear();
}

vector<list<int>> readInGraphAdjList(int* n, int* m)
{
    int u, v;

    if(scanf("%d", n)!=1)
    {
        fprintf(stderr, "problem with line 1 in input file\n");
        exit(1);
    }

    if(scanf("%d", m)!=1)
    {
        fprintf(stderr, "problem with line 2 in input file\n");
        exit(1);
    }

#ifdef DEBUG
    printf("Number of vertices: %d\n", *n);
    printf("Number of edges: %d\n", *m);
#endif

    vector<list<int>> adjList(*n);

    int i = 0;
    while(i < *m)
    {
        if(scanf("%d,%d", &u, &v)!=2)
        {
            printf("problem with line %d in input file\n", i+2);
            exit(1);
        }
        assert(u < *n && u > -1);
        assert(v < *n && v > -1);
        if(u==v)
            printf("%d=%d\n", u, v);
        assert(u != v);

        adjList[u].push_back(v);

        i++;
    }

#ifdef DEBUG
    printArrayOfLinkedLists(adjList, *n);
#endif

    return adjList;
}

vector<list<int>> readInGraphAdjListEdgesPerLine(int &n, int &m, string const &fileName)
{
    ifstream instream(fileName.c_str());

    if (instream.good() && !instream.eof()) {
        string line;
        std::getline(instream, line);
        while((line.empty() || line[0] == '%') && instream.good() && !instream.eof()) {
            std::getline(instream, line);
        }
        stringstream strm(line);
        strm >> n >> m;
    } else {
        fprintf(stderr, "ERROR: Problem reading number of vertices and edges in file %s\n", fileName.c_str());
        exit(1);
    }

#ifdef DEBUG
    printf("Number of vertices: %d\n", n);
    printf("Number of edges: %d\n", m);
#endif

    vector<list<int>> adjList(n);

    int u, v;
    int i = 0;
    while (i < n) {
        if (!instream.good()  || instream.eof()) {
            fprintf(stderr, "ERROR: Problem reading line %d in file %s\n", i+1, fileName.c_str());
            exit(1);
        }

        string line;
        std::getline(instream, line);
        u = i;
        stringstream strm(line);
        while (!line.empty() && strm.good() && !strm.eof()) {
            strm >> v;
            v--;

            assert(u < n && u > -1);
            assert(v < n && v > -1);
            if (u==v) {
                fprintf(stderr, "ERROR: Detected loop %d->%d\n", u + 1, v + 1);
                exit(1);
            }

            adjList[u].push_back(v);
        }

        i++;
    }

#ifdef DEBUG
    printArrayOfLinkedLists(adjList, n);
#endif

    return adjList;
}


vector<list<int>> readInGraphAdjList(int &n, int &m, string const &fileName)
{

    ifstream instream(fileName.c_str());

    if (instream.good() && !instream.eof())
        instream >> n;
    else {
        fprintf(stderr, "problem with line 1 in input file\n");
        exit(1);
    }


    if (instream.good() && !instream.eof())
        instream >> m;
    else {

        fprintf(stderr, "problem with line 2 in input file\n");
        exit(1);
    }

#ifdef DEBUG
    printf("Number of vertices: %d\n", n);
    printf("Number of edges: %d\n", m);
#endif

    vector<list<int>> adjList(n);

    int u, v;
    int i = 0;
    while(i < m)
    {
        char comma;
        if (instream.good() && !instream.eof()) {
            instream >> u >> comma >> v;
        } else {
            fprintf(stderr, "problem with line %d in input file\n", i+2);
            exit(1);
        }
        assert(u < n && u > -1);
        assert(v < n && v > -1);
        if(u==v)
            fprintf(stderr, "Detected loop %d->%d\n", u, v);
        assert(u != v);

        adjList[u].push_back(v);

        i++;
    }

#ifdef DEBUG
    printArrayOfLinkedLists(adjList, n);
#endif

    return adjList;
}

#if 0
vector<list<int>> readInGraphAdjListDimacs(int &n, int &m, string const &fileName)
{

    std::getline(instream, line);
    ifstream instream(fileName.c_str());

    if (instream.good() && !instream.eof())
        instream >> m;
    else {

        fprintf(stderr, "problem with line 2 in input file\n");
        exit(1);
    }

#ifdef DEBUG
    printf("Number of vertices: %d\n", n);
    printf("Number of edges: %d\n", m);
#endif

    vector<list<int>> adjList(n);

    int u, v; // endvertices, to read edges.
    int i = 0;
    while(i < m)
    {
        char comma;
        if (instream.good() && !instream.eof()) {
            instream >> u >> comma >> v;
        } else {
            fprintf(stderr, "problem with line %d in input file\n", i+2);
            exit(1);
        }
        assert(u < n && u > -1);
        assert(v < n && v > -1);
        if(u==v)
            fprintf(stderr, "Detected loop %d->%d\n", u, v);
        assert(u != v);

        adjList[u].push_back(v);

        i++;
    }

#ifdef DEBUG
    printArrayOfLinkedLists(adjList, n);
#endif

    return adjList;
}
#endif


void runAndPrintStatsMatrix(long (*function)(char**,
                                             int),
                            const char* algName,
                            char** adjMatrix,
                            int n )
{
    fprintf(stderr, "%s: ", algName);
    fflush(stderr);

    clock_t start = clock();

    long cliqueCount = function(adjMatrix, n);

    clock_t end = clock();

    fprintf(stderr, "%ld maximal cliques, ", cliqueCount);
    fprintf(stderr, "in %f seconds\n", (double)(end-start)/(double)(CLOCKS_PER_SEC));
    fflush(stderr);
}

void RunAndPrintStats(Algorithm *pAlgorithm, list<list<int>> &cliques, bool const outputLatex)
{
    fprintf(stderr, "%s: ", pAlgorithm->GetName().c_str());
    fflush(stderr);

    clock_t start = clock();

    long const cliqueCount = pAlgorithm->Run(cliques);

    clock_t end = clock();

    if (!outputLatex) {
        fprintf(stderr, "%ld maximal cliques, ", cliqueCount);
        fprintf(stderr, "in %f seconds\n", (double)(end-start)/(double)(CLOCKS_PER_SEC));
    } else {
        printf("%.2f", (double)(end-start)/(double)(CLOCKS_PER_SEC));
    }
    fflush(stderr);
}


void Tools::printList(list<int> const &linkedList, void (*printFunc)(int))
{
#ifdef DEBUG
    printf("printList...\n");
#endif
    int count = 0;
    const int listSize = static_cast<int>(linkedList.size());
    for (int const value : linkedList) {
        printFunc(value);
        if (++count != listSize) {
            printf(" ");
        }
    }

    printf("\n");

}

void printListAbbv(list<int> const &linkedList, void (*printFunc)(int))
{
#ifdef DEBUG
    printf("printListAbbv...\n");
#endif
    int count = 0;
    const int listSize = static_cast<int>(linkedList.size());

    for (list<int>::const_iterator cit = linkedList.begin();
         cit != linkedList.end() && count != 10; ++cit)
    {
        count++;
        printFunc(*cit);
        if(count != listSize)
        {
            printf(" ");
        }
    }

    if(count != listSize)
    {
        printf("... plus %d more", listSize - 10);
    }

    printf("\n");
}

void DescribeVertex(int const lineNumber, int *vertexSets, int *vertexLookup, int const size, int const vertex, int const beginX, int const beginD, int const beginP, int const beginR)
{
    (void)size;
    int const vertexLocation(vertexLookup[vertex]);

    cout << lineNumber << ": vertex " << vertex << " is in position " << vertexLocation << (vertexSets[vertexLocation] == vertex ? "(consistent)" : "(inconsistent: " + to_string(vertexSets[vertexLocation]) + " is there)" ) << " in set ";

    if (vertexLocation < beginX) {
        cout << "(before X)" << endl;
    }
    if (vertexLocation >= beginX && vertexLocation < beginD) {
        cout << "X" << endl;
    }
    if (vertexLocation >= beginD && vertexLocation < beginP) {
        cout << "D" << endl;
    }
    if (vertexLocation >= beginP && vertexLocation < beginR) {
        cout << "P" << endl;
    }
    if (vertexLocation >= beginR) {
        cout << "R" << endl;
    }
}

void DescribeSet(string const &setName, int const begin, int const end)
{
    cout << " " << setName << "=[" << begin << "->" << end << "]";
}

void DescribeState(int const lineNumber, int *vertexSets, int *vertexLookup, int const size, int const beginX, int const beginD, int const beginP, int const beginR)
{
    (void)vertexSets;
    (void)vertexLookup;
    cout << lineNumber << ": Size " << size;
    DescribeSet("X", beginX, beginD-1);
    DescribeSet("D", beginD, beginP-1);
    DescribeSet("P", beginP, beginR-1);
    DescribeSet("R", beginR, size-1);
    cout << endl << flush;
}

void CheckConsistency(int const lineNumber, size_t const recursionNumber, int *vertexSets, int *vertexLookup, int const size)
{
    for (int i=0; i < size; ++i) {
        if (vertexSets[vertexLookup[i]] != i) {
            cout << recursionNumber << "(line " << lineNumber << ") : inconsistency -- vertex " << i  << " is supposed to be in position " << vertexLookup[i] << " but vertex " <<  vertexSets[vertexLookup[i]] << " is there." << endl;
        }
    }
}

void CheckReverseConsistency(int const lineNumber, size_t const recursionNumber, int *vertexSets, int *vertexLookup, int const size)
{
    for (int i=0; i < size; ++i) {
        if (vertexLookup[vertexSets[i]] != i) {
            cout << recursionNumber << "(line " << lineNumber << ") : inconsistency -- vertex " << vertexSets[i]  << " is supposed to be in position " << vertexLookup[vertexSets[i]] << " but it is in position " << i << "." << endl;
        }
    }
}

void InvertGraph(vector<list<int>> const &adjList)
{
    int const n(adjList.size());
    cout << n << endl;
    size_t numEdgesInInverse(0);
    for (list<int> const &neighbors : adjList) {
        numEdgesInInverse += n - neighbors.size() - 1; // all non-edges except loops
    }

    cout << numEdgesInInverse << endl;

    for (int i = 0; i < static_cast<int>(adjList.size()); ++i) {
        set<int> setNeighbors;
        setNeighbors.insert(adjList[i].begin(), adjList[i].end());
        for (int neighbor=0; neighbor < static_cast<int>(adjList.size()); neighbor++) {
            if (setNeighbors.find(neighbor) == setNeighbors.end() && neighbor != i) {
                cout << "(" << i << "," << neighbor << i << ")" << endl;
            }
        }
    }
}

string Tools::GetTimeInSeconds(clock_t delta, bool const brackets) {
    stringstream strm;

    strm.precision(2);
    strm.setf(std::ios::fixed, std::ios::floatfield);
    if (brackets) {
        strm << "[" << (double)(delta)/(double)(CLOCKS_PER_SEC) << "s]";
    } else {
        strm << (double)(delta)/(double)(CLOCKS_PER_SEC) << "s";
    }
    return strm.str();
}

vector<int> Tools::ReadMetisOrdering(string const &fileName)
{
    ifstream instream(fileName.c_str());

    vector<int> ordering;

    int v;

    if (!instream.good()  || instream.eof()) {
        fprintf(stderr, "ERROR: Problem reading line 1 in file %s\n", fileName.c_str());
        exit(1);
    }

    while (instream.good() && !instream.eof()) {

        string line;
        std::getline(instream, line);
        stringstream strm(line);
        if (!line.empty() && strm.good() && !strm.eof()) {
            strm >> v;

            cout << "read: " << v << endl;

            assert(v > -1);

            ordering.push_back(v);
        };
    }

#ifdef DEBUG
    printArrayOfLinkedLists(adjList, n);
#endif

    return ordering;
}

