#pragma once

#include "Simulator.h"

#define INITIAL_STATE_MASK 0

class Analyzer : public Simulator
{
    std::vector<const Exit*> previousExit;
    std::vector<int> previousStateMask;
    std::vector<SolutionRecord> solutionRecordHolder;
    int componentCurrentIndex;

    static int depth;
    static int prevDepth;

public:
    Analyzer(Level* currentLevel);

    int getComponentCurrentIndex() const;

    void setComponentCurrentIndex(int index);

    void analyzeComponents();
    void analyzeComponent(Component& component, int stateMask);

    void preprocess();

    virtual void preAction(Cell* cell, int dir) const;
    virtual void postAction(Cell* cell, int dir) const;

    virtual void preOccupyAction(Cell* cell, int dir) const;
    virtual void postOccupyAction(Cell* cell, int dir) const;
    virtual void preRestoreAction(Cell* cell, int dir) const;
    virtual void postRestoreAction(Cell* cell, int dir) const;

    virtual bool reachedFinalCell(Cell* cell, int dir) const;
    virtual bool potentialSolution(Cell* cell, int dir) const;
    virtual void solutionFound(Cell* cell, int dir);

    void collectResults(const Exit* exit1, const Exit* exit2);
    void uncollectResults();
};
