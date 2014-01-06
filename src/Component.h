#pragma once

#include "Common.h"
#include "AbstractComponent.h"
#include <tuple>

// Component -> StateMask -> SolutionHead -> SolutionBody -> [ StateMask -> ... ]
struct SolutionHead
{
    int startX, startY, startDir;

    bool operator < (const SolutionHead& head) const;
};

struct SolutionBody
{
    int endX, endY, endDir;
    //std::string solution; // TODO: consider compressing (1 bit per decision)

    bool operator < (const SolutionBody& body) const;
};

class SolutionTree;

typedef std::tuple<int, SolutionHead, SolutionBody> SolutionRecord;

// Solution tree implementation
class BodyToTree
{
    friend class SolutionTree;

    std::map<SolutionBody, SolutionTree> bodyToTree;

public:

    SolutionTree* followBody(const SolutionBody& solutionBody);
};

class HeadToBody
{
    friend class SolutionTree;

    std::map<SolutionHead, BodyToTree> headToBody;

public:

    BodyToTree* followHead(const SolutionHead& solutionHead);
};

class SolutionTree
{
    std::map<int, HeadToBody> tree;
    int solutionCount;

public:

    SolutionTree();

    HeadToBody* followStateMask(const int stateMask);
    int getSolutionCount() const;
    void addSolution(const std::vector<SolutionRecord>& solution);
};

class Component : public AbstractComponent
{
    int occupied;
    SolutionTree solutions;
    // Arranged counter-clockwise along the perimeter
    std::vector<const Exit*> exits;
    std::vector<const Cell*> exitCells;

    std::stack<SolutionTree*> remainingSolutions;

public:
    Component();

    Component(const Component& c);

    int getOccupiedCount() const;
    int getSolutionCount() const;
    SolutionTree* getSolutions(); // TODO: reconsider
    const std::vector<const Exit*>& getExits() const;
    const std::vector<const Cell*>& getExitCells() const;
    const SolutionTree* getRemainingSolutions() const;
    const Exit* getExitByIndex(int index) const;
    const Cell* getExitCellByIndex(int index) const;
    int getIndexByExit(const Exit* exit) const;
    int getIndexByExitCell(const Cell* exitCell) const;
    int getFreeExitCellsMask() const;
    int getCurrentExitStateMask() const;
    int getCurrentExitCellStateMask() const;

    void addExit(const Exit* e);
    void addExitCell(const Cell* cell);
    //void addSolution(SolutionTree* newSolution);
    void chooseSolution(const int stateMask, const SolutionHead& head, const SolutionBody& body);
    void unchooseSolution();

    void incrementOccupied(int num = 1);
    void decrementOccupied(int num = 1);
};
