#pragma once

#include "Common.h"
#include "AbstractComponent.h"

class ComponentSolution;

class Path
{
    const Exit* start;
    const Exit* finish;

    int length;

public:

    Path();
    Path(const Exit* a, const Exit* b, int length);

    bool operator < (const Path& p) const;

    const Exit* getStart() const;
    const Exit* getFinish() const;
    int getLength() const;
};

typedef std::map<int, std::map<Path, ComponentSolution> > SolutionMap; // Index -> Path -> [Index -> Path] -> ...

class ComponentSolution
{
    SolutionMap solutions;

public:

    SolutionMap* getSolutions();
};

class Component : public AbstractComponent
{
    int occupied;
    SolutionMap solutions;
    int solutionCount;
    std::set<const Exit*> exits; // TODO: rewrite with std::vector
    std::set<const Cell*> exitCells;

    std::stack<SolutionMap*> remainingSolutions;

public:
    Component();

    Component(const Component& c);

    int getOccupiedCount() const;
    int getSolutionCount() const;
    const SolutionMap* getSolutions() const;
    const std::set<const Exit*>& getExits() const;
    const std::set<const Cell*>& getExitCells() const;
    const SolutionMap* getRemainingSolutions() const;
    const Exit* getExitByIndex(int index) const;
    const Cell* getExitCellByIndex(int index) const;
    int getIndexByExit(const Exit* exit) const;
    int getFreeExitCellsMask() const;
    int getCurrentStateMask() const;

    void addExit(const Exit* e);
    void addExitCell(const Cell* cell);
    void addSolution(SolutionMap* newSolution);
    void chooseSolution(const Path* chosenPath);
    void unchooseSolution();

    void incrementOccupied(int num = 1);
    void decrementOccupied(int num = 1);
};
