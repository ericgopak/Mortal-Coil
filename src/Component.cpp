#include "Common.h"
#include "Component.h"
#include "Colorer.h"
#include "Level.h"

#include <string>
#include <algorithm>

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

//size_t SolutionHead::operator ()(const SolutionHead& head) const
//{
//    return (((head.startY & 65535) << 15) ^ ((head.startX & 65535) << 2)) ^ head.startDir;
//}

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

//size_t SolutionBody::operator ()(const SolutionBody& body) const
//{
//    return (((body.endY & 65535) << 15) ^ ((body.endX & 65535) << 2)) ^ body.endDir;
//}

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

void SolutionTree::clear()
{
    tree.headToBody.clear();
    solutionCount = 0;
    startingSolutionCount = 0;
    endingSolutionCount = 0;
    mustBeBlockedMask = 0;
    mustBeFreeMask = 0;
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

    subtree->mustBeBlockedMask &= std::get<0>(record); // TODO: investigate - this does not seem alright to me...
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

//void SolutionTree::removeSolution(const std::vector<SolutionRecord>& solution, bool isStarting, bool isEnding)
//{
//    if (solution.size() == 0)
//    {
//        return;
//    }
//
//    const SolutionHead& head = std::get<2>(solution[0]);
//    const SolutionBody& body = std::get<3>(solution[0]);
//
//    auto& htb = tree.headToBody;
//    if (htb.find(head) != htb.end())
//    {
//        auto& btt = htb[head].bodyToTree;
//        if (btt.find(body) != btt.end())
//        {
//            auto& subtree = btt[body];
//            auto subsolution = std::vector<SolutionRecord>(solution.begin() + 1, solution.end());
//
//            subtree.removeSolution(subsolution, isStarting, isEnding);
//            solutionCount--;
//            startingSolutionCount -= isStarting;
//            endingSolutionCount -= isEnding;
//
//            if (subtree.solutionCount == 0) // No more subtrees left
//            {
//                btt.erase(body);
//            }
//        }
//
//        if (btt.size() == 0) // No more bodies left
//        {
//            htb.erase(head);
//        }
//    }
//}

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

//void SolutionTree::convertToRecords(std::vector<std::vector<SolutionRecord>>& solutions, int depth) const
std::vector<std::vector<SolutionRecord>> SolutionTree::convertToRecords() const
{
    std::vector<std::vector<SolutionRecord>> records;
    FOREACH_CONST(tree.headToBody, htb)
    {
        const SolutionHead& head = htb->first;
        FOREACH_CONST(htb->second.bodyToTree, btt)
        {
            const SolutionBody& body = btt->first;

            SolutionRecord record(body.mustBeBlockedMask, body.mustBeFreeMask, head, body);
            auto recordAsVector = std::vector<SolutionRecord>(1, record);
            
            auto nextRecords = btt->second.convertToRecords();
            
            if (nextRecords.size() == 0)
            {
                records.push_back(recordAsVector);
            }
            else
            {
                FOREACH(nextRecords, it)
                {
                    auto tmp = recordAsVector;
                    tmp.insert(tmp.end(), it->begin(), it->end());
                    records.push_back(tmp);
                }
            }
        }
    }

    return records;
}

std::set<const Exit*> SolutionTree::getAllInExits(Level* level) const
{
    std::set<const Exit*> inExits;

    FOREACH_CONST(tree.headToBody, htb)
    {
        const SolutionHead& head = htb->first;

        const Exit* inExit = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
        inExits.insert(inExit);

        FOREACH_CONST(htb->second.bodyToTree, btt)
        {
            const SolutionBody& body = btt->first;

            auto nextAllInExits = btt->second.getAllInExits(level);

            std::vector<const Exit*> tmp(inExits.size() + nextAllInExits.size());
            auto it = std::set_union(inExits.begin(), inExits.end(), nextAllInExits.begin(), nextAllInExits.end(), tmp.begin());
            tmp.resize(it - tmp.begin());
            inExits = std::set<const Exit*>(tmp.begin(), tmp.end());
        }
    }

    return inExits;
}

std::set<const Exit*> SolutionTree::getAllOutExits(Level* level) const
{
    std::set<const Exit*> outExits;

    FOREACH_CONST(tree.headToBody, htb)
    {
        const SolutionHead& head = htb->first;

        FOREACH_CONST(htb->second.bodyToTree, btt)
        {
            const SolutionBody& body = btt->first;
            
            const Exit* outExit = level->getCell(body.endY, body.endX)->getExit(body.endDir);

            outExits.insert(outExit);

            auto nextAllOutExits = btt->second.getAllOutExits(level);

            std::vector<const Exit*> tmp(outExits.size() + nextAllOutExits.size());
            auto it = std::set_union(outExits.begin(), outExits.end(), nextAllOutExits.begin(), nextAllOutExits.end(), tmp.begin());
            tmp.resize(it - tmp.begin());
            outExits = std::set<const Exit*>(tmp.begin(), tmp.end());
        }
    }

    return outExits;
}

Component::Component()
    : AbstractComponent()
    , occupied(0)
    //, currentStateMask(0)
    , portal(NULL)
    , originalSolutionsAssigned(false)
{
    // TODO: test performance
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
}

Component::Component(const Component& c)
    : AbstractComponent(c)
    , occupied(c.occupied)
    , startingSolutions(c.startingSolutions)
    , throughSolutions(c.throughSolutions)
    , endingSolutions(c.endingSolutions)
    , nonStartingSolutions(c.nonStartingSolutions)
    , nonEndingSolutions(c.nonEndingSolutions)
    , solutions(c.solutions)
    , exits(c.exits)
    //, currentStateMask(0)
    , portal(NULL)
    , originalSolutionsAssigned(false)
{
    // TODO: test performance
    exits.reserve(MAX_EXPECTED_COMPONENT_EXITS);
    exitCells.reserve(MAX_EXPECTED_COMPONENT_EXITS);
}

int Component::getOccupiedCount() const
{
    return occupied;
}

int Component::getTotalSolutionCount() const
{
    //return startingSolutions.getSolutionCount() + throughSolutions.getSolutionCount() + endingSolutions.getSolutionCount();
    return startingSolutions.getSolutionCount() + nonStartingSolutions.getSolutionCount();
}

SolutionTree* Component::getStartingSolutions()
{
    return &startingSolutions;
}

SolutionTree* Component::getThroughSolutions()
{
    return &throughSolutions;
}

SolutionTree* Component::getEndingSolutions()
{
    return &endingSolutions;
}

SolutionTree* Component::getNonStartingSolutions()
{
    return &nonStartingSolutions;
}

SolutionTree* Component::getNonEndingSolutions()
{
    return &nonEndingSolutions;
}

SolutionTree* Component::getSolutions()
{
    return &solutions;
}

int Component::getStartingSolutionCount() const
{
    return startingSolutions.getSolutionCount();
}

int Component::getThroughSolutionCount() const
{
    assert(throughSolutions.getSolutionCount() == nonStartingSolutions.getSolutionCount() - nonStartingSolutions.getEndingSolutionCount());
    return throughSolutions.getSolutionCount();
}

int Component::getEndingSolutionCount() const
{
    return startingSolutions.getEndingSolutionCount() + nonStartingSolutions.getEndingSolutionCount();
    //return endingSolutions.getSolutionCount();
}

int Component::getNonStartingSolutionCount() const
{
    return nonStartingSolutions.getSolutionCount();
}

int Component::getNonEndingSolutionCount() const
{
    return nonEndingSolutions.getSolutionCount();
}

int Component::getSolutionCount() const
{
    return solutions.getSolutionCount();
}

struct SolutionTrait
{
    int exit1;
    int exit2;
    MustBeBlockedMask mustBeBlockedMask;
    MustBeFreeMask mustBeFreeMask;

    bool operator < (const SolutionTrait& st) const
    {
        if (exit1 != st.exit1) return exit1 < st.exit1;
        if (exit2 != st.exit2) return exit2 < st.exit2;
        if (mustBeBlockedMask != st.mustBeBlockedMask) return mustBeBlockedMask < st.mustBeBlockedMask;
        return mustBeFreeMask < st.mustBeFreeMask;
    }
};

typedef std::vector<SolutionTrait> SolutionTraits;

size_t Component::getUniqueSolutionCount() const
{
    // TODO: finish
    std::set<SolutionTraits> res;

    auto s = solutions.convertToRecords();

    FOREACH_CONST(s, it)
    {
        SolutionTraits sts;
        FOREACH_CONST(*it, it2)
        {
            const auto& record = *it2;
            const auto& bmask = std::get<0>(record);
            const auto& fmask = std::get<1>(record);
            const auto& head = std::get<2>(record);
            const auto& body = std::get<3>(record);
            int exit1 = -1;
            int exit2 = -1;
            FOREACH_CONST(exits, e)
            {
                const Exit* e1 = *e;
                if (e1->getX() == head.startX && e1->getY() == head.startY && e1->getDir() == (head.startDir ^ 2))
                {
                    exit1 = getIndexByExit(e1);
                    break;
                }
            }

            FOREACH_CONST(exits, e)
            {
                const Exit* e2 = *e;
                if (e2->getX() == body.endX && e2->getY() == body.endY && e2->getDir() == body.endDir)
                {
                    exit2 = getIndexByExit(e2);
                    break;
                }
            }

            SolutionTrait st = {exit1, exit2, bmask, fmask};
            sts.push_back(st);
        }

        /*FOREACH_CONST(sts, z) printf("(%d,%d,%d,%d)-->", z->exit1, z->exit2, z->mustBeBlockedMask, z->mustBeFreeMask);
        printf("\n");*/

        res.insert(sts);
    }

    return res.size();
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
    int x = (int)std::distance(exits.begin(), std::find(exits.begin(), exits.end(), exit));

    return x < (int)exits.size() ? x : -1;
}

int Component::getIndexByExitCell(const Cell* exitCell) const
{
    // TODO: this must be slow! Rewrite!
    int x = (int)std::distance(exitCells.begin(), std::find(exitCells.begin(), exitCells.end(), exitCell));
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
    for (int i = 0; i < (int)exits.size(); i++)
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
    for (int i = 0; i < (int)exitCells.size(); i++)
    {
        const Cell* exitCell = getExitCellByIndex(i);
        if (exitCell->isFree() == false)
        {
            mask |= 1 << i;
        }
    }
    return mask;
}

//#include "Level.h" // TODO: remove
//
//void SolutionTree::getValidThroughSolutions(SolutionTree& res) const
//{
//    const SolutionTree* subtree = this;
//
//    FOREACH(subtree->tree.headToBody, htb)
//    {
//        const SolutionHead& head = htb->first;
//
//        const Cell* fromCell = Debug::level->getCell(head.startY, head.startX);
//        const Exit* fromExit = fromCell->getExit(head.startDir ^ 2);
//        const Exit* opposingExit = fromExit->getOpposingExit();
//
//        // Good if incoming
//
//        // Restricting if bidirectional
//
//        // Bad otherwise
//
//        int counter1 = 0;
//
//        //FOREACH(htb->second.bodyToTree, btt)
//        //{
//        //    const SolutionBody& body = btt->first;
//        //    int counter2 = 0;
//
//
//        //    // TODO: check if children got any solutions
//        //    // if child->solutionCount == 0 then remove it
//        //}
//    }
//
//    //HeadToBody* headToBody = &subtree->tree;
//    //BodyToTree* bodyToTree = &headToBody->headToBody[head];
//    //subtree = &bodyToTree->bodyToTree[body];
//
//    //addSolution(subtree, solution, solutionIndex + 1, isStarting, isEnding);
//
//    ////FOREACH()
//    //{
//    //}
//}

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

std::set<int> Component::getNeighbours() const
{
    std::set<int> ind;

    FOREACH_CONST(getExits(), e)
    {
        const auto& exit = *e;
        int compID = exit->getOpposingExit()->getHostCell()->getComponentId();
        ind.insert(compID);
    }

    return ind;
}

void Component::clearRemainingSolutions()
{
    remainingSolutions = std::stack<SolutionTree*>();
}

void Component::assignSolutions(Level* level, const SolutionList& solutionList)
{
    if (originalSolutionsAssigned == true)
    {
        originalSolutionsAssigned = false;

        originalStartingSolutions = startingSolutions;
        originalThroughSolutions = throughSolutions;
        originalEndingSolutions = endingSolutions;
        originalNonStartingSolutions = nonStartingSolutions;
        originalNonEndingSolutions = nonEndingSolutions;
        originalSolutions = solutions;
    }

    startingSolutions.clear();
    throughSolutions.clear();
    endingSolutions.clear();
    nonStartingSolutions.clear();
    nonEndingSolutions.clear();
    solutions.clear();

    for (int i = 0; i < (int)solutionList.size(); i++)
    {
        const auto& solution = solutionList[i];

        bool isStarting = true;
        bool isEnding = true;

        const Exit* exit1 = NULL;
        const Exit* exit2 = NULL;

        const auto& mustBeBlockedMask = std::get<0>(solution[0]);
        const auto& mustBeFreeMask = std::get<1>(solution[0]);

        const auto& head = std::get<2>(solution[0]);
        FOREACH_CONST(exits, e)
        {
            const Exit* e1 = *e;
            if (e1->getX() == head.startX && e1->getY() == head.startY && e1->getDir() == (head.startDir ^ 2))
            {
                exit1 = e1;
                if (!((1 << getIndexByExit(e1)) & mustBeBlockedMask))
                {
                    // Cannot be through. Leave it alone
                    break;
                }
                isStarting = false;
                break;
            }
        }

        const auto& body = std::get<3>(solution.back());
        FOREACH_CONST(exits, e)
        {
            const Exit* e2 = *e;
            if (e2->getX() == body.endX && e2->getY() == body.endY && e2->getDir() == body.endDir)
            {
                exit2 = e2;
                if ((1 << getIndexByExit(e2)) & mustBeBlockedMask)
                {
                    // Cannot be through. Leave it alone
                    break;
                }
                isEnding = false;
                break;
            }
        }

        if (exit1 && exit1->getHostCell()->getComponentId() == 289)
        {
            int bp = 0;
        }

        if (isStarting)
        {
            startingSolutions.addSolution(solution, isStarting, isEnding);
        }
        else
        {
            nonStartingSolutions.addSolution(solution, isStarting, isEnding);
            // Can also be starting?
            //if (!((1 << getIndexByExit(exit1)) & mustBeFreeMask))
            if (mustBeBlockedMask == 0)
            {
                startingSolutions.addSolution(solution, true, isEnding);
            }
        }

        if (isEnding)
        {
            endingSolutions.addSolution(solution, isStarting, isEnding);
        }
        else
        {
            nonEndingSolutions.addSolution(solution, isStarting, isEnding);
            // Can also be ending?
            if (!((1 << getIndexByExit(exit2)) & mustBeFreeMask))
            {
                endingSolutions.addSolution(solution, isStarting, true);
            }
        }

        if (!isStarting && !isEnding)
        {
            throughSolutions.addSolution(solution, isStarting, isEnding);
        }

        solutions.addSolution(solution, isStarting, isEnding);
/*
        // -----------------------------------------
        {
            //bool isStarting = true;

            //MustBeBlockedMask mustBeBlockedMask = std::get<0>(solution[0]);
            MustBeBlockedMask mustBeFreeMask = std::get<1>(solution[0]);

            const int& startX = std::get<2>(solution[0]).startX;
            const int& startY = std::get<2>(solution[0]).startY;
            const int& startDir = std::get<2>(solution[0]).startDir;

            const Cell* cell = level->getCell(startY, startX);
            //const Component* comp = &level->getComponents()[cell->getComponentId()];

            // Exit behind and this exit must not be free
            if (cell->hasExit(startDir ^ 2) && (mustBeFreeMask & (1 << getIndexByExit(cell->getExit(startDir ^ 2)))) == 0)
            {
                isStarting = false;
            }
        }

        //bool isEnding = true;
        {
            const SolutionRecord& lastRecord = solution[solution.size() - 1];
            const int& endX = std::get<3>(lastRecord).endX;
            const int& endY = std::get<3>(lastRecord).endY;
            const int& endDir = std::get<3>(lastRecord).endDir;

            MustBeBlockedMask mustBeFreeMask = std::get<1>(solution[solution.size() - 1]);

            const Cell* cell = level->getCell(endY, endX);
            const Component* comp = &level->getComponents()[cell->getComponentId()];

            // Exit in front and it must be free
            if (cell->hasExit(endDir) && (mustBeFreeMask & (1 << comp->getIndexByExit(cell->getExit(endDir)))) != 0)
            {
                isEnding = false;
            }
        }
        // ----------------------------------------------
*/

        //bool isStarting = solutionIsStarting(solutionRecordHolder);
        //bool isEnding = solutionIsEnding(solutionRecordHolder);
        //component.getSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
        //if (isStarting)
        //{
        //    startingSolutions.addSolution(solution, isStarting, isEnding);
        //}
        //else
        //{
        //    nonStartingSolutions.addSolution(solution, isStarting, isEnding);
        //}

        //if (isEnding)
        //{
        //    endingSolutions.addSolution(solution, isStarting, isEnding);
        //}
        //else
        //{
        //    nonEndingSolutions.addSolution(solution, isStarting, isEnding);
        //}

        //if (!isStarting && !isEnding)
        //{
        //    throughSolutions.addSolution(solution, isStarting, isEnding);
        //}

        //solutions.addSolution(solution, isStarting, isEnding);
    }
}

void Component::setSolutionsAsOriginal()
{
    originalStartingSolutions = startingSolutions;
    originalThroughSolutions = throughSolutions;
    originalEndingSolutions = endingSolutions;
    originalNonStartingSolutions = nonStartingSolutions;
    originalNonEndingSolutions = nonEndingSolutions;
    originalSolutions = solutions;

    originalSolutionsAssigned = true;
}

void Component::restoreOriginalSolutions()
{
    if (originalSolutionsAssigned)
    {
        return;
    }

    startingSolutions = originalStartingSolutions;
    throughSolutions = originalThroughSolutions;
    endingSolutions = originalEndingSolutions;
    nonStartingSolutions = originalNonStartingSolutions;
    nonEndingSolutions = originalNonEndingSolutions;
    solutions = originalSolutions;

    originalSolutionsAssigned = true;
}
