#include "Common.h"
#include "Component.h"
#include "Colorer.h"

#include <string>

bool SolutionHead::operator < (const SolutionHead& head) const
{
    if (startY != head.startY) return startY < head.startY;
    if (startX != head.startX) return startX < head.startX;
    return startDir < head.startDir;
}

bool SolutionHead::operator == (const SolutionHead& head) const
{
    return startY == head.startY
        && startX == head.startX
        && startDir == head.startDir;
}

size_t SolutionHead::operator ()(const SolutionHead& head) const
{
    return (((head.startY & 65535) << 15) ^ ((head.startX & 65535) << 2)) ^ head.startDir;
}

bool SolutionBody::operator < (const SolutionBody& body) const
{
    if (endY   != body.endY  ) return endY   < body.endY;
    if (endX   != body.endX  ) return endX   < body.endX;
    if (endDir != body.endDir) return endDir < body.endDir;
    if (mustBeBlockedMask != body.mustBeBlockedMask) return mustBeBlockedMask < body.mustBeBlockedMask;
    if (mustBeFreeMask    != body.mustBeFreeMask   ) return mustBeFreeMask    < body.mustBeFreeMask   ;
    if (stateChangeMask   != body.stateChangeMask  ) return stateChangeMask   < body.stateChangeMask  ;

    return solution < body.solution;
}

bool SolutionBody::operator == (const SolutionBody& body) const
{
    return endY   == body.endY
        && endX   == body.endX
        && endDir == body.endDir
        && mustBeBlockedMask == body.mustBeBlockedMask
        && mustBeFreeMask    == body.mustBeFreeMask
        && stateChangeMask   == body.stateChangeMask
        && solution          == body.solution;
}

size_t SolutionBody::operator ()(const SolutionBody& body) const
{
    return (((body.endY & 65535) << 15) ^ ((body.endX & 65535) << 2)) ^ body.endDir;
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

BodyToTree* SolutionTree::followHead(const SolutionHead& solutionHead)
{
    return tree.headToBody.find(solutionHead) == tree.headToBody.end()
        ? NULL
        : &tree.headToBody[solutionHead];
}

SolutionTree::SolutionTree()
    : solutionCount(0)
    , startingSolutionCount(0)
    , endingSolutionCount(0)
    // These masks are supposed to be AND-ed afterwards
    , mustBeBlockedMask(~0)
    , mustBeFreeMask(~0)
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

void SolutionTree::addSolution(SolutionTree* tree, const std::vector<SolutionRecord>& solution, int solutionIndex, bool isStarting, bool isEnding)
{
    if (solutionIndex >= (int)solution.size())
    {
        return;
    }

    SolutionTree* subtree = tree;

    subtree->solutionCount++;
    subtree->startingSolutionCount += isStarting;
    subtree->endingSolutionCount += isEnding;

    const SolutionRecord& record = solution[solutionIndex];

    subtree->mustBeBlockedMask &= std::get<0>(record);
    subtree->mustBeFreeMask    &= std::get<1>(record);

    HeadToBody* headToBody = &subtree->tree;
    BodyToTree* bodyToTree = &headToBody->headToBody[std::get<2>(record)];
    subtree = &bodyToTree->bodyToTree[std::get<3>(record)];

    addSolution(subtree, solution, solutionIndex + 1, isStarting, isEnding);
}

void SolutionTree::addSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding)
{
    assert(solution.size() > 0);

    addSolution(this, solution, 0, isStarting, isEnding);
}

std::vector<SolutionRecord> SolutionTree::firstSolutionToRecords() const
{
    std::vector<SolutionRecord> records;

    const SolutionTree* subtree = this;
    while (subtree->getSolutionCount() > 0)
    {
        const SolutionHead& head = subtree->tree.headToBody.begin()->first;
        const SolutionBody& body = subtree->tree.headToBody.begin()->second.bodyToTree.begin()->first;
        subtree = &subtree->tree.headToBody.begin()->second.bodyToTree.begin()->second;

        records.push_back(SolutionRecord(body.mustBeBlockedMask, body.mustBeFreeMask, head, body));
    }

    return records;
}

void SolutionTree::convertToRecords(std::vector<std::vector<SolutionRecord>>& solutions, int depth) const
{
    // Do we need this? Maybe just traverse the tree instead?
}

Component::Component()
    : AbstractComponent()
    , occupied(0)
    //, currentStateMask(0)
    , portal(NULL)
{
    // TODO: test performance
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    remainingSolutions.push(&startingSolutions);
}

Component::Component(const Component& c)
    : AbstractComponent(c)
    , occupied(c.occupied)
    , startingSolutions(c.startingSolutions)
    , nonStartingSolutions(c.nonStartingSolutions)
    , exits(c.exits)
    //, currentStateMask(0)
    , portal(NULL)
{
    // TODO: test performance
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    remainingSolutions.push(&nonStartingSolutions); // TODO: choose starting solutions for the first ones only
}

int Component::getOccupiedCount() const
{
    return occupied;
}

int Component::getTotalSolutionCount() const
{
    return startingSolutions.getSolutionCount() + nonStartingSolutions.getSolutionCount();
}

SolutionTree* Component::getNonStartingSolutions()
{
    return &nonStartingSolutions;
}

SolutionTree* Component::getStartingSolutions()
{
    return &startingSolutions;
}

SolutionTree* Component::getThroughSolutions()
{
    return &throughSolutions;
}

int Component::getNonStartingSolutionCount() const
{
    return nonStartingSolutions.getSolutionCount();
}

int Component::getStartingSolutionCount() const
{
    return startingSolutions.getSolutionCount();
}

int Component::getThroughSolutionCount() const
{
    assert(throughSolutions.getSolutionCount() == nonStartingSolutions.getSolutionCount() - nonStartingSolutions.getEndingSolutionCount());
    return nonStartingSolutions.getSolutionCount() - nonStartingSolutions.getEndingSolutionCount();
}

int Component::getEndingSolutionCount() const
{
    return startingSolutions.getEndingSolutionCount() + nonStartingSolutions.getEndingSolutionCount();
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

#include "Level.h" // TODO: remove

void SolutionTree::getValidThroughSolutions(SolutionTree& res) const
{
    const SolutionTree* subtree = this;

    FOREACH(subtree->tree.headToBody, htb)
    {
        const SolutionHead& head = htb->first;

        const Cell* fromCell = Debug::level->getCell(head.startY, head.startX);
        const Exit* fromExit = fromCell->getExit(head.startDir ^ 2);
        const Exit* opposingExit = fromExit->getOpposingExit();

        // Good if incoming

        // Restricting if bidirectional

        // Bad otherwise

        int counter1 = 0;

        //FOREACH(htb->second.bodyToTree, btt)
        //{
        //    const SolutionBody& body = btt->first;
        //    int counter2 = 0;


        //    // TODO: check if children got any solutions
        //    // if child->solutionCount == 0 then remove it
        //}
    }

    //HeadToBody* headToBody = &subtree->tree;
    //BodyToTree* bodyToTree = &headToBody->headToBody[head];
    //subtree = &bodyToTree->bodyToTree[body];

    //addSolution(subtree, solution, solutionIndex + 1, isStarting, isEnding);

    ////FOREACH()
    //{
    //}
}

const Portal* Component::getPortal() const
{
    return portal;
}

bool Component::isPortal() const
{
    return portal != NULL;
}

void Component::setPortal(const Portal* p)
{
    portal = p;
}

//int Component::getCurrentStateMask() const
//{
//    return currentStateMask;
//}
//
//void Component::setCurrentStateMask(int mask)
//{
//    currentStateMask = mask;
//}

//SolutionTree* Component::getSolutions()
//{
//    return &solutions;
//}

void Component::addExit(const Exit* e)
{
    exits.push_back(e);
    assert(exits.size() < MAX_EXPECTED_COMPONENT_EXITS);
}

void Component::addExitCell(const Cell* cell)
{
    exitCells.push_back(cell);
}

void Component::chooseSolution(SolutionTree* subtree)
{
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

//void Component::toggleExitState(const SolutionHead& head)
//{
//    // Find exit index and update currentStateMask
//    int index = -1;
//    for (size_t i = 0; i < exits.size(); i++)
//    {
//        if (exits[i]->getX() == head.startX && exits[i]->getY() && exits[i]->getDir() == head.startDir)
//        {
//            index = i;
//            break;
//        }
//    }
//    assert(index != -1);
//    currentStateMask ^= 1 << index;
//}
