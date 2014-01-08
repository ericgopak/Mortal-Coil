#include "Common.h"
#include "Component.h"
#include "Colorer.h"

bool SolutionHead::operator < (const SolutionHead& head) const
{
    if (startY != head.startY) return startY < head.startY;
    if (startX != head.startX) return startX < head.startX;
    return startDir < head.startDir;
}

bool SolutionBody::operator < (const SolutionBody& body) const
{
    if (endY != body.endY) return endY < body.endY;
    if (endX != body.endX) return endX < body.endX;
    return endDir < body.endDir;
}

SolutionTree* BodyToTree::followBody(const SolutionBody& solutionBody)
{
    return bodyToTree.find(solutionBody) == bodyToTree.end()
        ? NULL
        : &bodyToTree[solutionBody];
}

BodyToTree* HeadToBody::followHead(const SolutionHead& solutionHead)
{
    return headToBody.find(solutionHead) == headToBody.end()
        ? NULL
        : &headToBody[solutionHead];
}

HeadToBody* SolutionTree::followStateMask(const int stateMask)
{
    return tree.find(stateMask) == tree.end()
        ? NULL
        : &tree[stateMask];
}

SolutionTree::SolutionTree()
    : solutionCount(0)
    , startingSolutionCount(0)
    , endingSolutionCount(0)
{
}

int SolutionTree::getSolutionCount() const
{
    return solutionCount;
}

int SolutionTree::getStartingSolutionCount() const
{
    return startingSolutionCount;
}

int SolutionTree::getEndingSolutionCount() const
{
    return endingSolutionCount;
}

void SolutionTree::addSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding)
{
    assert(solution.size() > 0);

    // TODO: check for solution uniqueness.

    SolutionTree* subtree = this;
    for (size_t i = 0; i < solution.size(); i++)
    {
        subtree->solutionCount++;
        subtree->startingSolutionCount += isStarting;
        subtree->endingSolutionCount += isEnding;

        const SolutionRecord& record = solution[i];
        HeadToBody* headToBody = &subtree->tree[std::get<0>(record)];
        BodyToTree* bodyToTree = &headToBody->headToBody[std::get<1>(record)];
        subtree = &bodyToTree->bodyToTree[std::get<2>(record)];
    }
}

Component::Component()
    : AbstractComponent()
    , occupied(0)
    , currentStateMask(0)
{
    // TODO: test performance
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    remainingSolutions.push(&solutions);
}

Component::Component(const Component& c)
    : AbstractComponent(c)
    , occupied(c.occupied)
    , solutions(c.solutions)
    , exits(c.exits)
    , currentStateMask(0)
{
    // TODO: test performance
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
    return solutions.getSolutionCount();
}

const std::vector<const Exit*>& Component::getExits() const
{
    return exits;
}

const std::vector<const Cell*>& Component::getExitCells() const
{
    return exitCells;
}

SolutionTree* Component::getRemainingSolutions() const
{
    assert(remainingSolutions.size() > 0);
    return remainingSolutions.top();
}

const Exit* Component::getExitByIndex(int index) const
{
    return exits[index];
}

const Cell* Component::getExitCellByIndex(int index) const
{
    return exitCells[index];
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

int Component::getFreeExitsMask() const
{
    return (1 << exits.size()) - 1;
}

int Component::getFreeExitCellsMask() const
{
    return (1 << exitCells.size()) - 1;
}

int Component::getInnerExitStateMask() const
{
    int mask = 0;
    for (size_t i = 0; i < exits.size(); i++)
    {
        const Exit* exit = getExitByIndex(i);
if (exit->getHostCell() == Debug::INITIAL_CELL)
{
    continue; // UGLY HACK: DO NOT COUNT SPECIAL CASES
}

        if (exit->getHostCell()->isFree() == false)
        {
            mask |= 1 << i;
        }
    }
    return mask;
}

int Component::getOuterExitStateMask() const
{
    int mask = 0;
    for (size_t i = 0; i < exits.size(); i++)
    {
        const Exit* exit = getExitByIndex(i)->getOpposingExit();
        if (exit->getHostCell()->isFree() == false)
        {
            mask |= 1 << i;
        }
    }
    return mask;
}

int Component::getActualExitStateMask() const
{
    return getInnerExitStateMask() | getOuterExitStateMask();
}

int Component::getCurrentExitCellStateMask() const
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

int Component::getCurrentStateMask() const
{
    return currentStateMask;
}

void Component::setCurrentStateMask(int mask)
{
    currentStateMask = mask;
}

SolutionTree* Component::getSolutions()
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

void Component::chooseSolution(const int stateMask, const SolutionHead& head, const SolutionBody& body)
{
    SolutionTree* currentOptions = remainingSolutions.top();
    HeadToBody* headToBody = currentOptions->followStateMask(stateMask);
    assert(headToBody != NULL && "Failed to choose given solution: state mask not found!");
    BodyToTree* bodyToTree = headToBody->followHead(head);
    assert(bodyToTree != NULL && "Failed to choose given solution: solution head not found!");
    SolutionTree* subtree = bodyToTree->followBody(body);

    remainingSolutions.push(subtree);
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

void Component::toggleExitState(const SolutionHead& head)
{
    // Find exit index and update currentStateMask
    int index = -1;
    for (size_t i = 0; i < exits.size(); i++)
    {
        if (exits[i]->getX() == head.startX && exits[i]->getY() && exits[i]->getDir() == head.startDir)
        {
            index = i;
            break;
        }
    }
    assert(index != -1);
    currentStateMask ^= 1 << index;
}
