#pragma once

#include "Simulator.h"

class Solver : public Simulator
{
    const char* outputFilename;

public:
    Solver(Level* currentLevel, const char* outputFilename);

    virtual void preOccupyAction(Cell* cell, int dir) const;
    virtual void postOccupyAction(Cell* cell, int dir) const;
    virtual void preRestoreAction(Cell* cell, int dir) const;
    virtual void postRestoreAction(Cell* cell, int dir) const;
    virtual void preAction(Cell* cell, int dir) const;
    virtual void postAction(Cell* cell, int dir) const;

    virtual bool reachedFinalCell(Cell* cell, int dir) const;
    virtual bool potentialSolution(Cell* cell, int dir) const;
    virtual void solutionFound(Cell* cell, int dir);

    void solve(int row, int col, int firstComponentId);
    //void follow(const Exit* exit);
    void trySolving(int startX, int startY);
};
