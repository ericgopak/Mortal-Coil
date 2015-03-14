#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"
#include "Solver.h"

#include <sstream>
#include <iostream>
#include <iterator>

Solver::Solver(Level* currentLevel, const char* _outputFilename)
    : Simulator(currentLevel)
    , outputFilename(_outputFilename)
    , depth(0)
    , stepCounter(0)
    , cellsVisited(0)
    , endingComponentID(-1)
{
}

void Solver::preOccupyAction(Cell* cell, int dir)
{
    assert(cell->isFree());

    //touchObstacles(cell);

    TRACE(cell->setDepth(tracer.depth));
}

void Solver::postOccupyAction(Cell* cell, int dir)
{
    assert(cell->isFree() == false);

    addTemporaryEnds(cell, dir);
    addTemporaryEndBlocks(cell, dir);
}

void Solver::preRestoreAction(Cell* cell, int dir)
{
    assert(cell->isFree() == false);

    removeTemporaryEnds(cell, dir);
    removeTemporaryEndBlocks(cell, dir);
}

void Solver::postRestoreAction(Cell* cell, int dir)
{
    assert(cell->isFree());

    //untouchObstacles(cell);

    TRACE(cell->setDepth(-1));
}

void Solver::preAction(Cell* cell, int dir)
{
}

void Solver::postAction(Cell* cell, int dir)
{
}

//bool Solver::preAction(Cell* cell, int dir, bool flag)
//{
//    if (flag && makeTouch(cell, flag) == false)
//    {
//#ifdef TRACE_STATISTICS
//        Debug::invalidTouchDetected++;
//#endif
//        return false;
//    }
//
//    if (flag)
//    {
//        addTemporaryEnds(cell, dir);
//    }
//
//    return true;
//}
//
//bool Solver::postAction(Cell* cell, int dir, bool flag)
//{
//    if (!flag && makeTouch(cell, flag) == false)
//    {
//        assert(false && "Should be failure-free!");
//        return false;
//    }
//
//    if (!flag)
//    {
//        removeTemporaryEnds(cell, dir);
//    }
//
//    return true;
//}

bool Solver::reachedFinalCell(Cell* cell, int dir) const
{
    // If we are at the very last remaining cell
    return level->Free == 1;
}

bool Solver::potentialSolution(Cell* cell, int dir) const
{
    if (level->getTemporaryEnds().size() > 1)
    {
#ifdef TRACE_STATISTICS
        Debug::gotTooManyTemporaryEndsCounter++;
#endif
        return false;
    }

//    if (checkTouchingObstacles(cell) == false)
//    {
//#ifdef TRACE_STATISTICS
//        Debug::gotInvalidNextTouchesCounter++;
//#endif
//        return false;
//    }

    if (gotIsolatedCells(cell, dir))
    {
#ifdef TRACE_STATISTICS
        Debug::gotIsolatedCellsCounter++;
#endif
        return false;
    }

    if (level->getTemporaryEndBlocks().size() > 2)
    {
#ifdef TRACE_STATISTICS
        Debug::gotTooManyTemporaryEndBlocksCounter++;
#endif
        return false;
    }

    return true;
}

void Solver::solutionFound(Cell* cell, int dir)
{
    TRACE(Colorer::print<WHITE>("Solution found!!!\n"));
    level->Solved = true;
}

bool Solver::gotIsolatedCells(Cell* cell, int dir) const
{
    const Cell* front = cell->getNextCell(dir);
    const Cell* frontLeft = cell->getNextCell(dir)->getNextCell(Left[dir]);
    const Cell* frontRight = cell->getNextCell(dir)->getNextCell(Right[dir]);

    if (front->isObstacle() == false && front->isFree() == false ||
        frontLeft->isObstacle() == false && frontLeft->isFree() == false ||
        frontRight->isObstacle() == false && frontRight->isFree() == false)
    {
        const Cell* left  = cell->getNextCell(Left[dir]);
        const Cell* right = cell->getNextCell(Right[dir]);
        if (left->isFree() && right->isFree())
        {
            return true;
        }
    }
    return false;
}

void Solver::addTemporaryEnds(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* behind = cell->getNextCell(dir ^ 2);
    if (left->isTemporaryEnd())
    {
level->traceComponent();
        level->addTemporaryEnd(left);
    }
    if (right->isTemporaryEnd())
    {
level->traceComponent();
        level->addTemporaryEnd(right);
    }
    if (behind->isTemporaryEnd())
    {
level->traceComponent();
        level->addTemporaryEnd(behind);
    }
}

void Solver::removeTemporaryEnds(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* behind = cell->getNextCell(dir ^ 2);
    if (left->isTemporaryEnd())
    {
level->traceComponent();
        level->removeTemporaryEnd(left);
    }
    if (right->isTemporaryEnd())
    {
level->traceComponent();
        level->removeTemporaryEnd(right);
    }
    if (behind->isTemporaryEnd())
    {
level->traceComponent();
        level->removeTemporaryEnd(behind);
    }
}

void Solver::addTemporaryEndBlocks(Cell* cell, int dir) const
{
    // Assuming cell has just been occupied
    if (cell->hasExits())
    {
        const Component* comp = &level->getComponents()[cell->getComponentId()];

        if (comp->getSize() == 1) // Special case - these components do not affect temporary end blocks
        {
            return;
        }
        assert(comp->getExitCells().size() > 0 && "Assuming no components have 0 exits");

        int fullMask = comp->getFreeExitCellsMask();
        
        // Compare previous and current exit cell masks
        int currentMask = comp->getCurrentExitCellStateMask();

        if (currentMask == fullMask) // No exits remain.
        {
            level->removeTemporaryEndBlock(comp);
        }
        else if (__popcnt(currentMask ^ fullMask) == 1) // Only one exit remains
        {
            level->addTemporaryEndBlock(comp);
        }
    }
}

void Solver::removeTemporaryEndBlocks(Cell* cell, int dir) const
{
    // Assuming cell is just about to get unoccupied
    if (cell->hasExits())
    {
        const Component* comp = &level->getComponents()[cell->getComponentId()];

        if (comp->getSize() == 1) // Special case - these components do not affect temporary end blocks
        {
            return;
        }
        assert(comp->getExitCells().size() > 0 && "Assuming no components have 0 exits");

        int fullMask = comp->getFreeExitCellsMask();
        
        // Compare previous and current exit cell masks
        int currentMask = comp->getCurrentExitCellStateMask();

        if (currentMask == fullMask) // No exits remain.
        {
            level->addTemporaryEndBlock(comp);
        }
        else if (__popcnt(currentMask ^ fullMask) == 1) // Only one exit remains
        {
            level->removeTemporaryEndBlock(comp);
        }
    }
}

// *** This does not work, because isolated bits do not correspond to pits. It's hard to tell if the exit really is isolated ***
// TODO: consider exits to itself! See level23 (x=3,y=1)->RDL
//void Solver::addTemporaryEndBlocks(Cell* cell, int dir) const
//{
//    // Assuming cell has just been occupied
//    if (cell->hasExits())
//    {
//        const Component* comp = &level->getComponents()[cell->getComponentId()];
//
//        if (comp->getSize() == 1) // Special case - these components do not affect temporary end blocks
//        {
//            return;
//        }
//
//        int exitCellCount = comp->getExitCells().size();
//        int fullMask = comp->getFreeExitCellsMask();
//        
//        // Compare previous and current exit masks
//        int currentMask = comp->getCurrentStateMask();
//        int index = comp->getIndexByExitCell(cell);
//        int previousMask = currentMask ^ (1 << index);
//
//        int bit = previousMask ^ currentMask;
//        int leftBit = (bit << 1) < fullMask ? (bit << 1) : 1;
//        int leftLeftBit = (leftBit << 1) < fullMask ? (leftBit << 1) : 1;
//        int rightBit = (bit >> 1) > 0 ? (bit >> 1) : (1 << (exitCellCount - 1));
//        int rightRightBit = (rightBit >> 1) > 0 ? (rightBit >> 1) : (1 << (exitCellCount - 1));
//
//        if ((~previousMask & (leftBit | bit | rightBit)) == bit) // Exit got joined back. Neighbouring exits are occupied
//        {
//            level->removeTemporaryEndBlock(comp);
//        }
//        else
//        {
//            if ((~currentMask & (leftLeftBit | leftBit | bit)) == leftBit) // Exit to the left got separated
//            {
//                level->addTemporaryEndBlock(comp);
//            }
//
//            if (leftBit != rightBit) // May happen with 2 exitCells
//            {
//                if ((~currentMask & (bit | rightBit | rightRightBit)) == rightBit) // Exit to the right got separated
//                {
//                    level->addTemporaryEndBlock(comp);
//                }
//            }
//        }
//    }
//}

//void Solver::removeTemporaryEndBlocks(Cell* cell, int dir) const
//{
//    // Assuming cell is just about to get unoccupied
//    if (cell->hasExits())
//    {
//        const Component* comp = &level->getComponents()[cell->getComponentId()];
//
//        if (comp->getSize() == 1) // Special case - these components do not affect temporary end blocks
//        {
//            return;
//        }
//
//        int exitCellCount = comp->getExitCells().size();
//        int fullMask = comp->getFreeExitCellsMask();
//        
//        // Compare previous and current exit masks
//        int currentMask = comp->getCurrentStateMask();
//        int index = comp->getIndexByExitCell(cell);
//        int previousMask = currentMask ^ (1 << index);
//
//        int bit = previousMask ^ currentMask;
//        int leftBit = (bit << 1) < fullMask ? (bit << 1) : 1;
//        int leftLeftBit = (leftBit << 1) < fullMask ? (leftBit << 1) : 1;
//        int rightBit = (bit >> 1) > 0 ? (bit >> 1) : (1 << (exitCellCount - 1));
//        int rightRightBit = (rightBit >> 1) > 0 ? (rightBit >> 1) : (1 << (exitCellCount - 1));
//
//        if ((~previousMask & (leftBit | bit | rightBit)) == bit) // Exit got joined back. Neighbouring exits are occupied
//        {
//            level->addTemporaryEndBlock(comp);
//        }
//        else
//        {
//            if ((~currentMask & (leftLeftBit | leftBit | bit)) == leftBit) // Exit to the left got separated
//            {
//                level->removeTemporaryEndBlock(comp);
//            }
//
//            if (leftBit != rightBit) // May happen with 2 exitCells
//            {
//                if ((~currentMask & (bit | rightBit | rightRightBit)) == rightBit) // Exit to the right got separated
//                {
//                    level->removeTemporaryEndBlock(comp);
//                }
//            }
//        }
//    }
//}

void Solver::solve(int row, int col, int firstComponentId)
{
    const std::set<int>& specialComponentIds = level->getSpecialComponentIds();

    if (level->initialEnds.size() == 2)
    {
        TRACE(Colorer::print<RED>("Level has 2 ends!\n"));
        const Cell* e1 = level->initialEnds[0];
        const Cell* e2 = level->initialEnds[1];
        trySolving(e1->getX(), e1->getY());
        if (level->Solved)
        {
            return;
        }
        trySolving(e2->getX(), e2->getY());
        if (level->Solved)
        {
            return;
        }
        assert(level->Solved && "Failed to solve the level with 2 ends!");
    }
    else if (level->initialEnds.size() == 1)
    {
        TRACE(Colorer::print<RED>("Level has 1 end!\n"));
        const Cell* e = level->initialEnds[0];
        trySolving(e->getX(), e->getY());
        if (level->Solved)
        {
            return;
        }
        else
        {
            TRACE(Colorer::print<RED>("Failed to solve the level with 1 end!\n"));
        }
    }

    if (specialComponentIds.size() == 2)
    {
        // We know the first and the last components
        TRACE(Colorer::print<RED>("Level has 2 special components!\n"));

        for (int y = 1; y <= level->getHeight(); y++)
        {
            for (int x = 1; x <= level->getWidth(); x++)
            {
                // If cell belongs to special component
                if (specialComponentIds.find(level->getCell(y, x)->getComponentId()) != specialComponentIds.end())
                {
                    trySolving(x, y);
                    if (level->Solved)
                    {
                        return;
                    }
                }
            }
        }

        assert(level->Solved && "Failed to solve the level with 2 special components!");
    }
    else if (specialComponentIds.size() == 1)
    {
        // Try starting from it
        TRACE(Colorer::print<RED>("Level has 1 special component!\n"));

        for (int y = 1; y <= level->getHeight(); y++)
        {
            for (int x = 1; x <= level->getWidth(); x++)
            {
                // If cell belongs to special component
                if (specialComponentIds.find(level->getCell(y, x)->getComponentId()) != specialComponentIds.end())
                {
Colorer::print<WHITE>("Starting from the SPECIAL component %d at (%d, %d)...\n", level->getCell(y, x)->getComponentId(), x, y);
                    trySolving(x, y);
                    if (level->Solved)
                    {
                        return;
                    }
                }
            }
        }

        if (!level->Solved)
        {
            TRACE(Colorer::print<RED>("Failed to solve starting from the only special component!\n"));
        }
    }

    TRACE(Colorer::print<WHITE>("Trying all possible starting points...\n"));

    for (int startY = row; startY <= level->getHeight(); startY++)
    {
        for (int startX = col; startX <= level->getWidth(); startX++)
        {
            const Cell* cell = level->getCell(startY, startX);
            if (!cell->isObstacle())
            {
                //TRACE(printf("%d,%d\n", startX, startY));

                const Component& comp = level->getComponents()[cell->getComponentId()];

                if (firstComponentId == -1)
                {
                    trySolving(startX, startY);
                }
                else if (cell->getComponentId() == firstComponentId)
                {
                    TRACE(printf("(%d, %d) in component %d\n", startX, startY, cell->getComponentId()));
                    trySolving(startX, startY);
                }
                
                if (level->Solved)
                {
                    return;
                }
            }
        }
    }

    assert(level->Solved && "Failed to solve the level! Something's definitely wrong...");
}

void changeExitCellsState(Component* comp, bool block, StateMask stateChange)
{
    for (size_t i = 0; i < comp->getExitCells().size(); i++)
    {
        if (stateChange & (1 << i))
        {
            const_cast<Cell*>(comp->getExitCells()[i])->setFree(!block); // UGLY HACK!
        }
    }
}

bool Solver::makeTouch(Cell* cell, bool touch) const
{
    bool ok = true;
    std::set<Obstacle*> alreadyTouched;
    for (int dir = 0; dir < 8; dir++){
        const Cell* ncell = cell->getNextCell(dir);
        if (ncell->isObstacle() == false)
        {
            continue;
        }

        Obstacle* o = &level->getObstacles()[ncell->getObstacleId()];

        if (alreadyTouched.find(o) != alreadyTouched.end())
        {
            continue;
        }

        if (!touch)
        {
//Colorer::print<GREEN>("untouch %d  (%d,%d)\n", ncell->getObstacleId(), cell->getX(), cell->getY());
            o->untouch(cell);
        }
        else
        {
//Colorer::print<GREEN>("touch %d  (%d,%d)\n", ncell->getObstacleId(), cell->getX(), cell->getY());
            if (o->touch(cell) == false)
            {
                // Touch has been invalid - cells touched in incorrect order
                ok = false;
                break;
            }
        }

        alreadyTouched.insert(o);
    }

    if (!ok)
    {
        // Untouch all the touched obstacles
        FOREACH(alreadyTouched, it)
        {
            auto& o = *it;
//Colorer::print<GREEN>("RECOVER: untouch %d  (%d,%d)\n", o, cell->getX(), cell->getY());
            o->untouch(cell);
        }
    }

    return ok;
}

bool Solver::traversePath(Cell* cell, int dir, const char* path, bool flag)
{
static int calls = 0;
calls++;

//printf("Traversing... (%d,%d)  %d\n", cell->getX(), cell->getY(), flag);
//level->traceComponent();
//int bp = 0;
    // Special case: starting solution
    if (Direction[dir] == path[0]) path++;

    if (cell->hasExit(dir))
    {
        if (cell->getNextCell(dir)->isFree() == true)
        {
if (*path != 0)
{
    Debug::level->traceComponent();
}
            assert(*path == 0);

            if (flag && makeTouch(cell, flag) == false)
            {
#ifdef TRACE_STATISTICS
                Debug::invalidTouchDetected++;
#endif
                return false;
            }

            //if (!flag)
            //{
            //    removeTemporaryEnds(cell, dir);
            //}

            Debug::ctr1 += flag ? 1 : -1;
            assert(cell->isFree() != !flag);
            cell->setFree(!flag);
            //cell->setTag(flag ? depth : DEFAULT_TAG);
            cell->setTag(flag ? stepCounter : DEFAULT_TAG);
            stepCounter += flag ? 1 : -1;

            //if (flag)
            //{
            //    addTemporaryEnds(cell, dir);
            //}

            if (!flag && makeTouch(cell, flag) == false)
            {
                assert(false && "Supposed to be failure-free! (in order for post-recovery to work)");
            }

            return true;
        }
        // Otherwise proceed with turning
    }

    if (cell->getNextCell(dir)->isObstacle()
        || (flag && (cell->getNextCell(dir)->isFree() == false || cell->getNextCell(dir)->getTag() != DEFAULT_TAG))
        || (!flag && cell->getNextCell(dir)->getTag() != cell->getTag() + 1))
        /*||  cell->getNextCell(dir)->getTag() != DEFAULT_TAG && cell->getTag() != cell->getNextCell(dir)->getTag())*/
        //|| (!flag && (cell->getNextCell(dir)->getTag() == DEFAULT_TAG || cell->getTag() != cell->getNextCell(dir)->getTag())))
    {
        if (*path == 0) // Finished
        {
            if (flag && makeTouch(cell, flag) == false)
            {
                return false;
            }

            //if (!flag)
            //{
            //    removeTemporaryEnds(cell, dir);
            //}

            Debug::ctr1 += flag ? 1 : -1;
            assert(cell->isFree() != !flag);
            cell->setFree(!flag);
            //cell->setTag(flag ? depth : DEFAULT_TAG);
            cell->setTag(flag ? stepCounter : DEFAULT_TAG);
            stepCounter += flag ? 1 : -1;

            //if (flag)
            //{
            //    addTemporaryEnds(cell, dir);
            //}

            if (!flag && makeTouch(cell, flag) == false)
            {
                assert(false && "Supposed to be failure-free! (in order for post-recovery to work)");
            }

            return true;
        }

        // Change direction
        int newDir = -1;
        for (int i = 0; i < 4; i++)
        {
            if (path[0] == Direction[i])
            {
                newDir = i;
                break;
            }
        }

        return traversePath(cell, newDir, path + 1, flag);
    }

    if (flag && makeTouch(cell, flag) == false)
    {
        return false;
    }

    if (flag)
    {
        Debug::ctr1 += flag ? 1 : -1;
        assert(cell->isFree() != !flag);
        cell->setFree(!flag);
        //cell->setTag(flag ? depth : DEFAULT_TAG);
        cell->setTag(flag ? stepCounter : DEFAULT_TAG);
        stepCounter += flag ? 1 : -1;
    }
    
    //if (flag)
    //{
    //    addTemporaryEnds(cell, dir);
    //}

    bool res = traversePath(cell->getNextCell(dir), dir, path, flag);

    if (!flag)
    {
        Debug::ctr1 += flag ? 1 : -1;
        assert(cell->isFree() != !flag);
        cell->setFree(!flag);
        //cell->setTag(flag ? depth : DEFAULT_TAG);
        cell->setTag(flag ? stepCounter : DEFAULT_TAG);
        stepCounter += flag ? 1 : -1;
    }

    //if (!flag)
    //{
    //    removeTemporaryEnds(cell, dir);
    //}

    if (!flag && makeTouch(cell, flag) == false)
    {
        assert(false && "Supposed to be failure-free! (in order for post-recovery to work)");
    }

    if (res == false) // Unsuccessful, recover
    {
        assert(flag == true);

        makeTouch(cell, !flag);
        Debug::ctr1 += flag ? -1 : 1;
        assert(cell->isFree() != flag);
        cell->setFree(flag);
        //cell->setTag(!flag ? depth : DEFAULT_TAG);
        cell->setTag(!flag ? stepCounter : DEFAULT_TAG);
        stepCounter += !flag ? 1 : -1;
    }

    return res;
}

void Solver::follow(const SolutionHead& head)
{
#ifdef TRACE_STATISTICS
    Debug::numberOfCallsToFollow++;
#endif
    
    depth++;

    // Get opposing exit
    Cell* fromCell = level->getCell(head.startY, head.startX);
    int componentID = fromCell->getComponentId();
    Component* comp = &level->getComponents()[componentID];

    if (comp->isPortal())
    {
        // Try taking the portal
        //TODO: ...
    }

    SolutionTree* solutions = comp->getRemainingSolutions();

    const int outerExitsStateMask = comp->getOuterExitStateMask(); // TODO: this does not really work, see level 33 (x=13,y=9)

    BodyToTree* bodyToTree = solutions->followHead(head);
    if (bodyToTree == NULL)
    {
        // No solutions
        int bp = 0;
    }
    else
    {
        FOREACH(bodyToTree->bodyToTree, btt)
        {
            const SolutionBody& body = btt->first;

            if ((body.mustBeBlockedMask & outerExitsStateMask) != body.mustBeBlockedMask)
            {
                continue;
            }

            if ((body.mustBeFreeMask & outerExitsStateMask) != 0)
            {
                continue;
            }

            SolutionTree* subtree = &btt->second;

            bool iveBeenTheEndingOne = false;

            if (subtree->getSolutionCount() > 0 && subtree->getSolutionCount() == subtree->getEndingSolutionCount())
            {
                if (endingComponentID == -1)
                {
                    endingComponentID = fromCell->getComponentId();
                    iveBeenTheEndingOne = true;
                }
                else if (endingComponentID != componentID)
                {
                    // Only ending solutions left, pruning
#ifdef TRACE_STATISTICS
                    Debug::endingOnlySolutionsDetected++;
#endif
                    continue;
                }
            }

            TRACE(printf("Trying (%d, %d, %d) --> (%d, %d, %d)\n", head.startX, head.startY, head.startDir, body.endX, body.endY, body.endDir));

            Cell* toCell = level->getCell(body.endY, body.endX);

#ifdef PRUNE_FOLLOW
            int prevStepCounter = stepCounter;
            if (traversePath(fromCell, head.startDir, body.solution.c_str(), true))
            {
#endif

            const StateMask originalInnerState = comp->getCurrentExitCellStateMask();
            StateMask toBeChangedMask = ~originalInnerState & body.stateChangeMask;
            changeExitCellsState(comp, true, toBeChangedMask);

#ifdef TRACE_SOLUTIONS // TODO: uncomment
            //fromCell->setFree(false); // WARNING: touching this may result in undefined behaviour
            //toCell->setFree(false);
//comp->setPortal((const Portal*)1); // just a temporary visualizing tool

static int BEST = 0;
if (cellsVisited > BEST)
{
    //printf("BETTER -> %d (comp --> %d)\n", cellsVisited, toCell->getComponentId());
    level->traceComponent();
    printf("Trying (%d, %d, %d) --> (%d, %d, %d)\n", head.startX, head.startY, head.startDir, body.endX, body.endY, body.endDir);
    //level->traceComponent(toCell->getComponentId());
    //system("pause");
    //BEST = cellsVisited;
    int bp = 0;
}
#endif

            if (subtree->getSolutionCount() == 0) // Current component traversed completely
            {
                cellsVisited += comp->getSize();

                if (cellsVisited == level->Free)
                {
Colorer::print<WHITE>("SOLUTION FOUND!!! Ended in component %d\n", toCell->getComponentId());
level->traceComponent();
level->traceComponent(toCell->getComponentId());
                    level->Solved = true;

                    level->Answer = body.solution + level->Answer;

                    depth--;
                    return;
                }
            }

            comp->chooseSolution(subtree);

            int dir = body.endDir;
            SolutionHead nextHead = {body.endX + dx[dir], body.endY + dy[dir], dir};

            if (level->getCell(nextHead.startY, nextHead.startX)->isFree())
            {
                follow(nextHead);
                if (level->Solved)
                {
                    level->Answer = body.solution + level->Answer;
                    depth--;
                    return;
                }
            }

#ifdef TRACE_SOLUTIONS // TODO: uncomment
            //fromCell->setFree(true); // WARNING: touching this may result in undefined behaviour
            //toCell->setFree(true);
comp->setPortal(NULL); // just a temporary visualizing tool
#endif

            changeExitCellsState(comp, false, toBeChangedMask);
                
            comp->unchooseSolution();

            if (subtree->getSolutionCount() == 0)
            {
                cellsVisited -= comp->getSize();
            }

#ifdef PRUNE_FOLLOW
            traversePath(fromCell, head.startDir, body.solution.c_str(), false);
            assert(stepCounter == prevStepCounter);
            }
#endif

            if (iveBeenTheEndingOne)
            {
                endingComponentID = -1;
            }
        }
    }

    depth--;
}

void Solver::trySolving(int startX, int startY)
{
    Colorer::print<WHITE>("Trying to solve from (%d,%d)\n", startX, startY);

    Cell* cell = level->getCell(startY, startX);
    Component* comp = &level->getComponents()[cell->getComponentId()];

    for (int d = 0; d < 4; d++)
    {
        //if (cell->getX() != 4 || cell->getY() != 13 || d != 1) // TODO: remove
        //    continue;

        SolutionHead head = {startX, startY, d};
        if (comp->getStartingSolutionCount() > 0)
        {
            comp->chooseSolution(comp->getStartingSolutions());

            assert(Debug::ctr1 == 0);

            follow(head);
            if (level->Solved)
            {
                level->setSolutionStartXY(startX, startY);
                return;
            }

            assert(Debug::ctr1 == 0);

            comp->unchooseSolution();
        }
    }
}
