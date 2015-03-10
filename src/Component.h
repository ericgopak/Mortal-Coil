#pragma once

#include "Common.h"
#include "AbstractComponent.h"
#include <tuple>

#include "Cell.h"

// Outer-exit masks of a component
typedef int StateMask;
typedef StateMask MustBeBlockedMask;
typedef StateMask MustBeFreeMask;

struct SolutionHead;
struct SolutionBody;
struct BodyToTree;
struct HeadToBody;
class SolutionTree;

// TODO: first 2 masks don't seem to be required - remove
typedef std::tuple<MustBeBlockedMask, MustBeFreeMask, SolutionHead, SolutionBody> SolutionRecord;

typedef std::vector<std::vector<SolutionRecord>> SolutionList;
typedef std::map<int, SolutionList> SolutionListMap;
typedef std::map<int, SolutionTree> SolutionMap;

// Component -> StateMask -> SolutionHead -> SolutionBody -> [ StateMask -> ... ]
struct SolutionHead
{
    int startX, startY, startDir;

    bool operator < (const SolutionHead& head) const;
    bool operator == (const SolutionHead& head) const;

    //size_t operator ()(const SolutionHead& head) const;
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

    //size_t operator ()(const SolutionBody& body) const;
};

// Solution tree implementation
struct BodyToTree
{
    friend class SolutionTree;

    //std::map<SolutionBody, SolutionTree, SolutionBody> bodyToTree;
    std::map<SolutionBody, SolutionTree> bodyToTree;

public:

    SolutionTree* followBody(const SolutionBody& solutionBody);
};

struct HeadToBody
{
    friend class SolutionTree;

    std::map<SolutionHead, BodyToTree> headToBody;
    //std::map<SolutionHead, BodyToTree, SolutionHead> headToBody;

public:

    BodyToTree* followHead(const SolutionHead& solutionHead);
};

class SolutionTree
{
    HeadToBody tree;
    int solutionCount;
    int startingSolutionCount;
    int endingSolutionCount;

    MustBeBlockedMask mustBeBlockedMask;
    MustBeFreeMask    mustBeFreeMask;

public:

    SolutionTree();

    void clear();

    BodyToTree* followHead(const SolutionHead& solutionHead);
    int getSolutionCount() const;
    int getStartingSolutionCount() const;
    int getEndingSolutionCount() const;

    //void getValidThroughSolutions(SolutionTree& res) const;

    void addSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding);
    void addSolution(SolutionTree* tree, const std::vector<SolutionRecord>& solution, int solutionIndex, bool isStarting, bool isEnding);

    //void removeSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding);

    std::vector<SolutionRecord> firstSolutionToRecords() const;
    std::vector<std::vector<SolutionRecord>> convertToRecords() const;

    std::set<const Exit*> getAllInExits(Level* level) const;
    std::set<const Exit*> getAllOutExits(Level* level) const;
};

class Component : public AbstractComponent
{
    int occupied;
    SolutionTree startingSolutions;
    SolutionTree throughSolutions;
    SolutionTree endingSolutions;
    SolutionTree nonStartingSolutions;
    SolutionTree nonEndingSolutions;
    SolutionTree solutions;

    bool originalSolutionsAssigned;

    SolutionTree originalStartingSolutions;
    SolutionTree originalThroughSolutions;
    SolutionTree originalEndingSolutions;
    SolutionTree originalNonStartingSolutions;
    SolutionTree originalNonEndingSolutions;
    SolutionTree originalSolutions;

    // Arranged counter-clockwise along the perimeter
    std::vector<const Exit*> exits;
    // Arranged clockwise along the perimeter
    std::vector<const Cell*> exitCells;

    std::stack<SolutionTree*> remainingSolutions;

    //int currentStateMask;

    const Portal* portal;

public:
    Component();

    Component(const Component& c);

    int getOccupiedCount() const;
    int getTotalSolutionCount() const;
    SolutionTree* getStartingSolutions();
    SolutionTree* getThroughSolutions();
    SolutionTree* getEndingSolutions();
    SolutionTree* getNonStartingSolutions();
    SolutionTree* getNonEndingSolutions();
    SolutionTree* getSolutions();
    int getStartingSolutionCount() const;
    int getThroughSolutionCount() const;
    int getEndingSolutionCount() const;
    int getNonStartingSolutionCount() const;
    int getNonEndingSolutionCount() const;
    int getSolutionCount() const;
    
    int getUniqueSolutionCount() const;

    const std::vector<const Exit*>& getExits() const;
    const std::vector<const Cell*>& getExitCells() const;
    SolutionTree* getRemainingSolutions() const;
    const Exit* getExitByIndex(int index) const;
    const Cell* getExitCellByIndex(int index) const;
    int getIndexByExit(const Exit* exit) const;
    int getIndexByExitCell(const Cell* exitCell) const;
    int getFreeExitsMask() const;
    int getFreeExitCellsMask() const;
    int getOuterExitStateMask() const;
    int getCurrentExitCellStateMask() const;

    const Portal* getPortal() const;
    bool isPortal() const;

    void setPortal(const Portal* p);

    //int getCurrentStateMask() const;
    //void setCurrentStateMask(int mask);

    void addExit(const Exit* e);
    void addExitCell(const Cell* cell);
    void chooseSolution(SolutionTree* subtree);
    void unchooseSolution();

    void incrementOccupied(int num = 1);
    void decrementOccupied(int num = 1);

    //void toggleExitState(const SolutionHead& head);

    std::set<int> getNeighbours() const;

    void clearRemainingSolutions();
    void assignSolutions(Level* level, const SolutionList& solutionList);
    void setSolutionsAsOriginal();
    void restoreOriginalSolutions();
};
