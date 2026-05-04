#ifndef _DJS_MEMORYMANAGER_H_
#define _DJS_MEMORYMANAGER_H_
#include <cstdlib>
#include <algorithm>
#ifdef MEMORY_DEBUG
#include <cstdio.h>

static void* MallocWithCheck(size_t x)
{
#ifdef ALLOW_ALLOC_ZERO_BYTES
    void* retvalue = malloc(x);
#else
    void* retvalue = malloc(std::max(x, (size_t)1));
#endif

    if(retvalue==NULL)
    {
        fprintf(stderr, "ERROR, malloc returned null pointer, that means we probably ran out of memory...\n");
        exit(1);
    }

    return retvalue;
};

static void* CallocWithCheck(size_t x, size_t y)
{
#ifdef ALLOW_ALLOC_ZERO_BYTES
    void* retvalue = calloc(x,y);
#else
    void* retvalue = calloc(std::max(x, (size_t)1), std::max(y, (size_t)1));
#endif

    if(retvalue==NULL)
    {
        fprintf(stderr, "ERROR, calloc returned null pointer, that means we probably ran out of memory...\n");
        exit(1);
    }

    return retvalue;
};

#define Malloc(x) MallocWithCheck(x)
#define Calloc(x,y) CallocWithCheck(x,y)
#define Free(x) free(x)

#else

#ifdef ALLOW_ALLOC_ZERO_BYTES
#define Malloc(x) malloc(x)
#define Calloc(x,y) calloc(x,y)
#define Free(x) free(x)

#else

#define Malloc(x) malloc(std::max((size_t)(x), (size_t)1))
#define Calloc(x,y) calloc(std::max((size_t)(x), (size_t)1), std::max((size_t)(y), (size_t)1))
#define Free(x) free(x)

#endif // ALLOW_ALLOC_ZERO_BYTES
#endif // MEMORY_DEBUG

#endif

