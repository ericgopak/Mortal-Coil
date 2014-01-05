#define _CRT_SECURE_NO_WARNINGS
#pragma once

// Solution to the problem "Mortal Coil" at http://www.hacker.org/coil
// Author: Eric Gopak

// Main idea: Trying all possibilities with as much pruning as possible
// ------------------------ Improvements: ------------------------
// -- Counts 'ends': doesn't consider cases with > 2 'ends'
// -- Checks for pits beforehand (if one exists, then it's either head or tail)
// -- Splits obstacles into components (use all 8 directions for it)
// -- Optimized Solve() function
// -- Observation: if a cell has two neighbours with 2 neighbours each - ignore that cell
// -- Observation: every obstacle sides must be touched continuosly (otherwise free cells are separated)
// -- Marking cells as free/not free
// -- Every two neighbouring cells have a bond
// -- Every cell has a mask of neighbours (1 - neighbour is free, 0 - otherwise)
// -- Counting number of 'edges' - stop if there are not ehough edges left for the solutions (Edges < EdgesToBe)
// -- Global optimization

// Optimization ideas:
// -- Reorder exits in Component::exits in CW or CCW order (e.g. use atan2()?)
//      then filter solutions if pits appear (exits are split into separated sets)

//#define TRACE_STATISTICS

#ifdef _DEBUG
#   define TRACE(x) x;
#else
#   define TRACE(x)
#endif

#define MAX_BOARD_SIDE 200 // Max dimension of the board
#define MAX_SOLUTION_LENGTH 1000000 // Max length of the solution
#define MAX_EXPECTED_COMPONENTS 50000 // Max number of components
#define MAX_EXPECTED_COMPONENT_EXITS 20 // Max number of exits in a single component

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

#include "Cell.h"

const int dx[8] = {1, 0,-1, 0,  1,-1,-1, 1};
const int dy[8] = {0, 1, 0,-1,  1, 1,-1,-1};
const int Left[8]  = {3, 0, 1, 2, 0, 1, 2, 3};
const int Right[8] = {1, 2, 3, 0, 1, 2, 3, 0};
const char Direction[4] = {'R', 'D', 'L', 'U'};

const int BitCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4}; // Bits[i] = Number of set bits in i
const int P[4] = {1, 2, 4, 8}; // TODO: really?

namespace Debug
{
    extern int depth;
    extern bool traceFlag;
    extern int mostSolutions;
    extern int gotIsolatedCellsCounter;
}
