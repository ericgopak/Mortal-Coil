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

typedef std::map<Path, ComponentSolution> SolutionMap;

class ComponentSolution
{
    SolutionMap solutions;

public:

    SolutionMap* getSolutions();
};

class Component : public AbstractComponent
{
    int occupied;
    //SolutionMap solutions;
    SolutionMap solutions;
    int solutionCount;
    std::set<const Exit*> exits;

    std::stack<SolutionMap*> remainingSolutions;

public:
    Component();

    Component(const Component& c);

    int getOccupiedCount() const;
    int getSolutionCount() const;
    const SolutionMap* getSolutions() const;
    const std::set<const Exit*>& getExits() const;
    const SolutionMap* getRemainingSolutions() const;
    const Exit* getExitByIndex(int index) const;
    int getFreeExitsMask() const;

    void addExit(const Exit* e);
    void addSolution(SolutionMap* newSolution);
    void chooseSolution(const Path* chosenPath);
    void unchooseSolution();

    void incrementOccupied(int num = 1);
    void decrementOccupied(int num = 1);
};
