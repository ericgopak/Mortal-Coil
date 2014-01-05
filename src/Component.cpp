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
    // TODO: test
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    remainingSolutions.push(&solutions);
}

Component::Component(const Component& c)
    : AbstractComponent(c)
    , occupied(c.occupied)
    , solutionCount(c.solutionCount)
    , solutions(c.solutions)
    , exits(c.exits)
{
    // TODO: test
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
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

const std::vector<const Exit*>& Component::getExits() const
{
    return exits;
}

const std::vector<const Cell*>& Component::getExitCells() const
{
    return exitCells;
}

const SolutionMap* Component::getRemainingSolutions() const
{
    assert(remainingSolutions.size() > 0);
    return remainingSolutions.top();
}

const Exit* Component::getExitByIndex(int index) const
{
    // TODO: implement exits using std::vector
    assert(index < (int)exits.size());
    // std::advance() is linear for std::set
    auto it = exits.begin();
    std::advance(it, index);
    return *it;
}

const Cell* Component::getExitCellByIndex(int index) const
{
    // TODO: implement exits using std::vector
    assert(index < (int)exitCells.size());
    // std::advance() is linear for std::set
    auto it = exitCells.begin();
    std::advance(it, index);
    return *it;
}

int Component::getIndexByExit(const Exit* exit) const
{
    // TODO: this must be slow! Rewrite!
    size_t x = std::distance(exits.begin(), std::find(exits.begin(), exits.end(), exit));
    assert(x < exits.size() && "Exit not found!");

    return x;
}

int Component::getIndexByExitCell(const Cell* exitCell) const
{
    // TODO: this must be slow! Rewrite!
    size_t x = std::distance(exitCells.begin(), std::find(exitCells.begin(), exitCells.end(), exitCell));
    assert(x < exitCells.size() && "ExitCell not found!");

    return x;
}

int Component::getFreeExitCellsMask() const
{
    return (1 << exitCells.size()) - 1;
}

int Component::getCurrentStateMask() const
{
    int mask = 0;
    for (size_t i = 0; i < exitCells.size(); i++)
    {
        const Cell* exitCell = getExitCellByIndex(i);
        if (exitCell->isFree() == false)
        {
            mask |= 1 << i;
        }
    }
    return mask;
}

const SolutionMap* Component::getSolutions() const
{
    return &solutions;
}

void Component::addExit(const Exit* e)
{
    exits.push_back(e);
    assert(exits.size() < MAX_EXPECTED_COMPONENT_EXITS);
}

void Component::addExitCell(const Cell* cell)
{
    exitCells.push_back(cell);
}

// Assuming 'solution' contains only one solution
void Component::addSolution(SolutionMap* newSolution)
{
    // Caution: don't read the code. It just works =)
    // TODO: refactor

    SolutionMap* currentSolutions = &solutions;

    while (newSolution->size() > 0)
    {
        assert(newSolution->size() <= 1 && "Trying to add invalid solution!");

        SolutionMap::iterator newIndexToPath = newSolution->begin();
        SolutionMap::iterator currentIndexToPath = currentSolutions->find(newIndexToPath->first);
        if (currentIndexToPath == currentSolutions->end())
        {
            // New solution (different indexes)
            solutionCount++;
            (*currentSolutions)[newIndexToPath->first] = newIndexToPath->second;
            break;
        }
        else
        {
            assert(newIndexToPath->second.size() <= 1 && "Trying to add invalid solution!");

            const Path& newPath = newIndexToPath->second.begin()->first;
            std::map<Path, ComponentSolution>::iterator currentPathToSolution = currentIndexToPath->second.find(newPath);
            if (currentPathToSolution == currentIndexToPath->second.end())
            {
                // New solution (different paths)
                solutionCount++;

                (*currentSolutions)[newIndexToPath->first][newPath] = newIndexToPath->second.begin()->second;
                break;
            }
            else
            {
                newSolution = newIndexToPath->second.begin()->second.getSolutions();
                currentSolutions = currentIndexToPath->second.begin()->second.getSolutions();
            }
        }
    }
}

void Component::chooseSolution(const Path* chosenPath)
{
    SolutionMap* currentOptions = remainingSolutions.top();
    //SolutionMap::iterator it = currentOptions->find(*chosenPath);

    int currentStateMask = getCurrentStateMask();
    std::map<Path, ComponentSolution>::iterator it = (*currentOptions)[currentStateMask].find(*chosenPath);

    assert(it != (*currentOptions)[currentStateMask].end() && "Tried to choose non-existent path!");

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
