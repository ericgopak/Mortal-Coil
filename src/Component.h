#pragma once

#include "Common.h"
#include "AbstractComponent.h"
#include <tuple>

#include "Cell.h"

// Outer-exit masks of a component
typedef int StateMask;
typedef StateMask MustBeBlockedMask;
typedef StateMask MustBeFreeMask;

// Component -> StateMask -> SolutionHead -> SolutionBody -> [ StateMask -> ... ]
struct SolutionHead
{
    int startX, startY, startDir;

    bool operator < (const SolutionHead& head) const;
    bool operator == (const SolutionHead& head) const;

    size_t operator ()(const SolutionHead& head) const;
};

struct SolutionBody
{
    int endX, endY, endDir;
    MustBeBlockedMask mustBeBlockedMask;
    MustBeFreeMask mustBeFreeMask;
    StateMask stateChangeMask;
    std::string solution; // TODO: consider compressing (1 bit per decision)

    bool operator < (const SolutionBody& body) const;
    bool operator == (const SolutionBody& body) const;

    size_t operator ()(const SolutionBody& body) const;
};

class SolutionTree;

//typedef std::tuple<int, SolutionHead, SolutionBody> SolutionRecord;
typedef std::tuple<MustBeBlockedMask, MustBeFreeMask, SolutionHead, SolutionBody> SolutionRecord;

// Solution tree implementation
struct BodyToTree
{
    friend class SolutionTree;

    std::map<SolutionBody, SolutionTree> bodyToTree;
    //std::unordered_map<SolutionBody, SolutionTree> bodyToTree;

public:

    SolutionTree* followBody(const SolutionBody& solutionBody);
};

struct HeadToBody
{
    friend class SolutionTree;

    //std::map<SolutionHead, BodyToTree> headToBody;
    std::unordered_map<SolutionHead, BodyToTree, SolutionHead> headToBody;

public:

    BodyToTree* followHead(const SolutionHead& solutionHead);
};

class SolutionTree
{
    //std::map<int, HeadToBody> tree;
    HeadToBody tree;
    int solutionCount;
    int startingSolutionCount;
    int endingSolutionCount;

    MustBeBlockedMask mustBeBlockedMask;
    MustBeFreeMask    mustBeFreeMask;

public:

    SolutionTree();

    //HeadToBody* followStateMask(const int stateMask);
    BodyToTree* followHead(const SolutionHead& solutionHead);
    int getSolutionCount() const;
    int getStartingSolutionCount() const;
    int getEndingSolutionCount() const;
    void addSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding);
    void addSolution(SolutionTree* tree, const std::vector<SolutionRecord>& solution, int solutionIndex, bool isStarting, bool isEnding);
};

class Component : public AbstractComponent
{
    int occupied;
    SolutionTree solutions;
    // Arranged counter-clockwise along the perimeter
    std::vector<const Exit*> exits;
    std::vector<const Cell*> exitCells;

    std::stack<SolutionTree*> remainingSolutions;

    int currentStateMask;

public:
    Component();

    Component(const Component& c);

    int getOccupiedCount() const;
    int getSolutionCount() const;
    SolutionTree* getSolutions(); // TODO: reconsider
    const std::vector<const Exit*>& getExits() const;
    const std::vector<const Cell*>& getExitCells() const;
    SolutionTree* getRemainingSolutions() const;
    const Exit* getExitByIndex(int index) const;
    const Cell* getExitCellByIndex(int index) const;
    int getIndexByExit(const Exit* exit) const;
    int getIndexByExitCell(const Cell* exitCell) const;
    int getFreeExitsMask() const;
    int getFreeExitCellsMask() const;
    //int getInnerExitStateMask() const;
    int getOuterExitStateMask() const;
    //int getActualExitStateMask() const;
    int getCurrentExitCellStateMask() const;

    int getCurrentStateMask() const;
    void setCurrentStateMask(int mask);

    void addExit(const Exit* e);
    void addExitCell(const Cell* cell);
    //void addSolution(SolutionTree* newSolution);
    //void chooseSolution(const int stateMask, const SolutionHead& head, const SolutionBody& body);
    void chooseSolution(const SolutionHead& head, const SolutionBody& body);
    void unchooseSolution();

    void incrementOccupied(int num = 1);
    void decrementOccupied(int num = 1);

    void toggleExitState(const SolutionHead& head);
};
