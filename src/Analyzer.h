#pragma once

#include "Simulator.h"

#define INITIAL_STATE_MASK 0

class Analyzer : public Simulator
{
    //std::vector<const Exit*> previousExit;
    std::vector<const SolutionHead> previousHead;
    std::vector<MustBeBlockedMask> mustBeBlockedStateMask; // Mask of must-be-previously-blocked exits for current solution fragment
    std::vector<MustBeFreeMask> mustBeFreeStateMask; // Mask of must-be-previously-blocked exits for current solution fragment
    std::vector<StateMask> stateChangeStack; // Mask of blocked exit cells
    std::vector<SolutionRecord> solutionRecordHolder;
    std::vector<std::string> decisionHolder;
    int componentCurrentIndex;

    static int depth;
    static int prevDepth;

public:
    Analyzer(Level* currentLevel);

    int getComponentCurrentIndex() const;

    void setComponentCurrentIndex(int index);

    void analyzeComponents();
    //void analyzeComponent(Component& component, int stateMask);
    void analyzeComponent(Component& component);

    void preprocess();

    void backtrack(Cell* cell, int dir);

    virtual void preAction(Cell* cell, int dir);
    virtual void postAction(Cell* cell, int dir);

    virtual void preOccupyAction(Cell* cell, int dir);
    virtual void postOccupyAction(Cell* cell, int dir);
    virtual void preRestoreAction(Cell* cell, int dir);
    virtual void postRestoreAction(Cell* cell, int dir);

    virtual bool reachedFinalCell(Cell* cell, int dir) const;
    virtual bool potentialSolution(Cell* cell, int dir) const;
    virtual void solutionFound(Cell* cell, int dir);

    void proceedAnalyzing(Cell* cell, int dir);

    void collectResults(const SolutionHead& head, const SolutionBody& body);
    void uncollectResults();
};
