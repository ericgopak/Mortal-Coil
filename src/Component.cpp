#include "Common.h"
#include "Component.h"
#include "Colorer.h"

Path::Path()
    : start(NULL)
    , finish(NULL)
{
}

Path::Path(const Exit* a, const Exit* b, int length)
    : start(a)
    , finish(b)
    , length(length)
{
}

bool Path::operator < (const Path& p) const
{
    if (start != p.start)
    {
        return start < p.start;
    }
    return finish < p.finish;
}

const Exit* Path::getStart() const
{
    return start;
}

const Exit* Path::getFinish() const
{
    return finish;
}

int Path::getLength() const
{
    return length;
}

SolutionMap* ComponentSolution::getSolutions()
{
    return &solutions;
}

Component::Component()
    : AbstractComponent()
    , occupied(0)
    , solutionCount(0)
{
    remainingSolutions.push(&solutions);
}

Component::Component(const Component& c)
    : AbstractComponent(c)
    , occupied(c.occupied)
    , solutionCount(c.solutionCount)
    , solutions(c.solutions)
    , exits(c.exits)
{
    remainingSolutions.push(&solutions);
}

int Component::getOccupiedCount() const
{
    return occupied;
}

int Component::getSolutionCount() const
{
    return solutionCount;
}

const std::set<const Exit*>& Component::getExits() const
{
    return exits;
}

const SolutionMap* Component::getRemainingSolutions() const
{
#ifdef DEBUG
    if (remainingSolutions.size() == 0) throw std::exception("Incorrect usage of remainingSolutions");
#endif
    return remainingSolutions.top();
}

const SolutionMap* Component::getSolutions() const
{
    return &solutions;
}

void Component::addExit(const Exit* e)
{
    exits.insert(e);
}

// Assuming 'solution' contains only one solution
void Component::addSolution(SolutionMap* newSolution)
{
    while (newSolution->size() > 0)
    {
        SolutionMap::iterator p = newSolution->begin();
        SolutionMap::const_iterator it = solutions.find(p->first);
        if (it == solutions.cend())
        {
            // New solution (differs from existing)
            solutionCount++;
            solutions[p->first] = p->second;
            break;
        }
        else
        {
            newSolution = p->second.getSolutions();
        }
    }
}

void Component::chooseSolution(const Path* chosenPath)
{
    SolutionMap* currentOptions = remainingSolutions.top();
    SolutionMap::iterator it = currentOptions->find(*chosenPath);
    if (it == currentOptions->end())
    {
        throw std::exception("Tried to choose non-existent path!");
    }
    else
    {
        remainingSolutions.push(it->second.getSolutions());
    }
}

void Component::unchooseSolution()
{
    remainingSolutions.pop();
#ifdef DEBUG
    if (remainingSolutions.size() == 0) throw std::exception("Incorrect usage of remainingSolutions");
#endif
}

void Component::incrementOccupied(int num)
{
    occupied += num;
}

void Component::decrementOccupied(int num)
{
    occupied -= num;
}
