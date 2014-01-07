#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define TRACE_STATISTICS

#ifdef _DEBUG
#   define TRACE_SOLUTIONS
#endif

#ifdef TRACE_SOLUTIONS
#   define TRACE(x) x;
#else
#   define TRACE(x)
#endif

#define MAX_EXPECTED_COMPONENTS 50000 // Max number of components
#define MAX_EXPECTED_COMPONENT_EXITS 30 // Max number of exits in a single component
//#define MAX_EXPECTED_SOLUTION_COUNT 21280 // Max number of unique solutions of a component

#define FOREACH(x, it) for (decltype((x).begin()) it = (x).begin(); it != (x).end(); it++)
#define FOREACH_CONST(x, it) for (decltype((x).cbegin()) it = (x).cbegin(); it != (x).cend(); it++)

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <assert.h>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <algorithm>

const int dx[8] = {1, 0,-1, 0,  1,-1,-1, 1};
const int dy[8] = {0, 1, 0,-1,  1, 1,-1,-1};
const int Left[8]  = {3, 0, 1, 2, 0, 1, 2, 3};
const int Right[8] = {1, 2, 3, 0, 1, 2, 3, 0};
const char Direction[4] = {'R', 'D', 'L', 'U'};

const int BitCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4}; // Bits[i] = Number of set bits in i
const int P[4] = {1, 2, 4, 8}; // TODO: really?

class Level;

namespace Debug
{
    extern const Level* level;

    extern int depth;
    extern bool traceFlag;

#ifdef TRACE_STATISTICS
    extern int mostSolutions;
    extern int totalSolutions;
    extern int gotTooManyTemporaryEndsCounter;
    extern int gotInvalidNextTouchesCounter;
    extern int gotIsolatedCellsCounter;
    extern int gotTooManyTemporaryEndBlocksCounter;
#endif
#ifdef TRACE_SOLUTIONS
    extern int currentX;
    extern int currentY;
    extern int currentDir;
    /*extern int currentComponent;*/
#endif
}
