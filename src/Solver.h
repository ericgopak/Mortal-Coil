#pragma once

#include "Simulator.h"

class Solver : public Simulator
{
    const char* outputFilename;

public:
    Solver(Level* currentLevel, const char* outputFilename);

    //void Solve(Cell* cell, int from);
    void solve(int row, int col, int firstComponentId);
    void follow(const Exit* exit);
    void trySolving(int startX, int startY);

    virtual bool reachedFinalCell(Cell* cell, int dir) const;
    //virtual bool potentialSolution(Cell* cell, int dir) const;
    virtual void solutionFound(Cell* cell, int dir);
};
