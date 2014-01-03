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
    assert(remainingSolutions.size() > 0);
    return remainingSolutions.top();
}

const Exit* Component::getExitByIndex(int index) const
{
    assert(index < (int)exits.size());
    // std::advance() is linear for std::set
    auto it = exits.begin();
    std::advance(it, index);
    return *it;
}

int Component::getFreeExitsMask() const
{
    return (1 << exits.size()) - 1;
}

const SolutionMap* Component::getSolutions() const
{
    return &solutions;
}

void Component::addExit(const Exit* e)
{
    exits.insert(e);
    assert(exits.size() < MAX_EXPECTED_COMPONENT_EXITS);
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

    assert(it != currentOptions->end() && "Tried to choose non-existent path!");

    remainingSolutions.push(it->second.getSolutions());
}

void Component::unchooseSolution()
{
    remainingSolutions.pop();
    assert(remainingSolutions.size() > 0); // The base solution should be left intact
}

void Component::incrementOccupied(int num)
{
    occupied += num;
}

void Component::decrementOccupied(int num)
{
    occupied -= num;
}
