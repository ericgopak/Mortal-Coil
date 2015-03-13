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
    , cellsVisited(0)
{
}

void Solver::preOccupyAction(Cell* cell, int dir)
{
    assert(cell->isFree());

    touchObstacles(cell);

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

    untouchObstacles(cell);

    TRACE(cell->setDepth(-1));
}

void Solver::preAction(Cell* cell, int dir)
{
}

void Solver::postAction(Cell* cell, int dir)
{
    if (level->Solved)
    {
        // Accumulate answer
        level->prependSolutionCell(cell, dir);
    }
}

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

    if (checkTouchingObstacles(cell) == false)
    {
#ifdef TRACE_STATISTICS
        Debug::gotInvalidNextTouchesCounter++;
#endif
        return false;
    }

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
        level->addTemporaryEnd(left);
    }
    if (right->isTemporaryEnd())
    {
        level->addTemporaryEnd(right);
    }
    if (behind->isTemporaryEnd())
    {
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
        level->removeTemporaryEnd(left);
    }
    if (right->isTemporaryEnd())
    {
        level->removeTemporaryEnd(right);
    }
    if (behind->isTemporaryEnd())
    {
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

static void makeTouch(Cell* cell, bool touch)
{
    // TODO: implement
}

static bool updateTouchingComponents(Cell* cell, int dir, const char* path, int depth, bool touch)
{
    // Special case: starting solution
    if (Direction[dir] == path[0]) path++;

    if (cell->hasExit(dir))
    {
        if (cell->getNextCell(dir)->isFree() == true)
        //if (cell->getNextCell(dir)->isFree() == !touch) // Finished traversing
        //if (cell->getNextCell(dir)->isFree() == touch) // Finished traversing
        {
if (*path != 0)
{
    Debug::level->traceComponent();
}
            assert(*path == 0);

Debug::ctr1 += touch ? 1 : -1;
            cell->setFree(!touch);
            cell->setTag(touch ? depth : DEFAULT_TAG);
            makeTouch(cell, touch);

            return true;
        }
//        else if (*path == 0) // Seems to duplicate behaviour on turn
//        {
//Debug::ctr1 += touch ? 1 : -1;
//            cell->setFree(!touch);
//            cell->setTag(touch ? depth : DEFAULT_TAG);
//            makeTouch(cell, touch);
//
//            return true;
//        }
        // Otherwise proceed with turning
    }

    //if (!(cell->getNextCell(dir)->isObstacle() || cell->getComponentId() == cell->getNextCell(dir)->getComponentId()))
    //{
    //    Debug::level->traceComponent();
    //}
    //assert(cell->getNextCell(dir)->isObstacle() || cell->getComponentId() == cell->getNextCell(dir)->getComponentId());

    /*if (cell->getX() == 9 && cell->getY() == 2)
    {
Debug::level->traceComponent();
        int bp = 0;
    }*/

    if (cell->getNextCell(dir)->isObstacle()
        || cell->getNextCell(dir)->isFree() == !touch
        || cell->getNextCell(dir)->getTag() != DEFAULT_TAG && cell->getTag() != cell->getNextCell(dir)->getTag())
    {
        if (*path == 0) // Finished
        {
Debug::ctr1 += touch ? 1 : -1;
            cell->setFree(!touch);
            cell->setTag(touch ? depth : DEFAULT_TAG);
            makeTouch(cell, touch);
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

        //if (cell->hasExit(newDir) && cell->getNextCell(newDir)->isFree()) // Finished traversing
        //{
        //    assert(path == NULL);
        //    return true;
        //}

        return updateTouchingComponents(cell, newDir, path + 1, depth, touch);
    }
    //TODO... still need to figure out why 9, 2 goes back to 9, 1 before turning left (solution: LUL)
    makeTouch(cell, touch);
Debug::ctr1 += touch ? 1 : -1;
    cell->setFree(!touch);
    cell->setTag(touch ? depth : DEFAULT_TAG);
    bool res = updateTouchingComponents(cell->getNextCell(dir), dir, path, depth, touch);

    if (res == false) // Unsuccessful, recover
    {
        makeTouch(cell, !touch);
Debug::ctr1 += touch ? -1 : 1;
        cell->setFree(touch);
        cell->setTag(!touch ? depth : DEFAULT_TAG);
    }

    return res;
}

//#undef PRUNE_FOLLOW // TODO: remove!

void Solver::follow(const SolutionHead& head)
{
static int depth = 0;
    
    depth++;

    // Get opposing exit
    Cell* fromCell = level->getCell(head.startY, head.startX);

    Component* comp = &level->getComponents()[fromCell->getComponentId()];

    if (comp->isPortal())
    {
        // Try taking the portal
        //TODO: ...
    }



    SolutionTree* solutions = comp->getRemainingSolutions();

    // Does not work - we can only find out if a component will be ending
    // (it makes this component 'special' starting from this move)
//    if (solutions->getSolutionCount() == solutions->getEndingSolutionCount())
//    {
//        // Only ending solutions remained - check if this truly is the last component
//        if (level->Free - cellsVisited > comp->getSize())
//        {
//#ifdef TRACE_STATISTICS
//            Debug::avoidedEndingSolutionCounter++;
//#endif
//            return;
//        }
//    }

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

            TRACE(printf("Trying (%d, %d, %d) --> (%d, %d, %d)\n", head.startX, head.startY, head.startDir, body.endX, body.endY, body.endDir));

            Cell* toCell = level->getCell(body.endY, body.endX);

#ifdef PRUNE_FOLLOW
            if (updateTouchingComponents(fromCell, head.startDir, body.solution.c_str(), depth, true))
            {
#endif

            const StateMask originalInnerState = comp->getCurrentExitCellStateMask();
            StateMask toBeChangedMask = ~originalInnerState & body.stateChangeMask;
            changeExitCellsState(comp, true, toBeChangedMask);

#ifdef TRACE_SOLUTIONS // TODO: uncomment
            //fromCell->setFree(false); // WARNING: touching this may result in undefined behaviour
            //toCell->setFree(false);
//comp->setPortal((const Portal*)1); // just a temporary visualizing tool

static int BEST = 100;
if (cellsVisited > BEST){
    printf("BETTER -> %d (comp --> %d)\n", cellsVisited, toCell->getComponentId());
level->traceComponent();
printf("Trying (%d, %d, %d) --> (%d, %d, %d)\n", head.startX, head.startY, head.startDir, body.endX, body.endY, body.endDir);
level->traceComponent(toCell->getComponentId());
system("pause");
BEST = cellsVisited;
}
#endif

            if (subtree->getSolutionCount() == 0) // Current component traversed completely
            {
                cellsVisited += comp->getSize();

                if (cellsVisited == level->Free) // TODO: consider using sort of a constant here // Assumption: cells are not being occupied in follow()
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

#ifdef PRUNE_FOLLOW
            updateTouchingComponents(fromCell, head.startDir, body.solution.c_str(), depth, false);
            }
#endif

            if (subtree->getSolutionCount() == 0)
            {
                cellsVisited -= comp->getSize();
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

            //cell->setFree(false);
            assert(Debug::ctr1 == 0);

            follow(head);
            if (level->Solved)
            {
                level->setSolutionStartXY(startX, startY);
                return;
            }

            assert(Debug::ctr1 == 0);

            //cell->setFree(true);

            comp->unchooseSolution();
        }
    }
}
