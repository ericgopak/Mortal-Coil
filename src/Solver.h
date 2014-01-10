#pragma once

#include "Simulator.h"

class Solver : public Simulator
{
    const char* outputFilename;

    int cellsVisited;

public:
    Solver(Level* currentLevel, const char* outputFilename);

    virtual void preOccupyAction(Cell* cell, int dir);
    virtual void postOccupyAction(Cell* cell, int dir);
    virtual void preRestoreAction(Cell* cell, int dir);
    virtual void postRestoreAction(Cell* cell, int dir);
    virtual void preAction(Cell* cell, int dir);
    virtual void postAction(Cell* cell, int dir);

    virtual bool reachedFinalCell(Cell* cell, int dir) const;
    virtual bool potentialSolution(Cell* cell, int dir) const;
    virtual void solutionFound(Cell* cell, int dir);

    bool gotIsolatedCells(Cell* cell, int dir) const;
    void addTemporaryEnds(Cell* cell, int dir) const;
    void removeTemporaryEnds(Cell* cell, int dir) const;
    void addTemporaryEndBlocks(Cell* cell, int dir) const;
    void removeTemporaryEndBlocks(Cell* cell, int dir) const;

    void solve(int row, int col, int firstComponentId);
    void follow(const SolutionHead& head);
    void trySolving(int startX, int startY);
};
