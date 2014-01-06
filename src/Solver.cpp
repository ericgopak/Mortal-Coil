#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"
#include "Solver.h"

Solver::Solver(Level* currentLevel, const char* _outputFilename)
    : Simulator(currentLevel)
    , outputFilename(_outputFilename)
{
}

void Solver::preOccupyAction(Cell* cell, int dir) const
{
    assert(cell->isFree());

    touchObstacles(cell);

    TRACE(cell->setDepth(tracer.depth));
}

void Solver::postOccupyAction(Cell* cell, int dir) const
{
    assert(cell->isFree() == false);

    addTemporaryEnds(cell, dir);
    addTemporaryEndBlocks(cell, dir);
}

void Solver::preRestoreAction(Cell* cell, int dir) const
{
    assert(cell->isFree() == false);

    removeTemporaryEnds(cell, dir);
    removeTemporaryEndBlocks(cell, dir);
}

void Solver::postRestoreAction(Cell* cell, int dir) const
{
    assert(cell->isFree());

    untouchObstacles(cell);

    TRACE(cell->setDepth(-1));
}

void Solver::preAction(Cell* cell, int dir) const
{
}

void Solver::postAction(Cell* cell, int dir) const
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
            level->prepareSolution(e1->getX(), e1->getY());
            return;
        }
        trySolving(e2->getX(), e2->getY());
        if (level->Solved)
        {
            level->prepareSolution(e2->getX(), e2->getY());
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
            level->prepareSolution(e->getX(), e->getY());
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
                        level->prepareSolution(x, y);
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
                        level->prepareSolution(x, y);
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
                    level->prepareSolution(startX, startY);
                    return;
                }
            }
        }
    }

    assert(level->Solved && "Failed to solve the level! Something's definitely wrong...");
}

//void Solver::follow(const Exit* exit)
//{
////TODO: check if can actually move to that component
////TODO: check if final component (count fully-traversed components, if only one left untraversed, and current component has some free cells left - then we are trying to finish)
////TODO: think of accumulating answer (add sequence of Cell* to every Path, nay?)
//
////TODO: figure out how to start and how to terminate
//    // Get opposing exit
//    int dir = exit->getDir();
//    const Exit* opposingExit = exit->getNextCell(dir)->getExit(dir ^ 2); // TODO: consider saving a pointer in Exit class
//
//    if (opposingExit->isFree() == false)
//    {
//        return;
//    }
//
//    Component* compFrom = &level->getComponents()[exit->getComponentId()];
//    Component* compTo = &level->getComponents()[opposingExit->getComponentId()];
//
//    //if (compFrom->getSolutionCount() == 0)
//    //{
//    //    // Must be an 'end'
//    //    if (level->Free == 1)
//    //    {
//    //        printf("LAST END!\n");
//    //    }
//    //    else
//    //    {
//    //        printf("FIRST END???\n");
//    //    }
//    //}
//    //
//    //if (compTo->getSolutionCount() == 0)
//    //{
//    //    // Must be an 'end'
//    //    if (level->Free == 1)
//    //    {
//    //        printf("LAST END???\n");
//    //    }
//    //    else
//    //    {
//    //        printf("FIRST END!\n");
//    //    }
//    //}
//
//    const SolutionMap* solutions = compTo->getSolutions();
//    FOREACH(*solutions, it)
//    {
//        if (*it->first.getStart() == *opposingExit)
//        {
//            //printf("Following (%d, %d, %d)...\n", opposingExit->getX(), opposingExit->getY(), opposingExit->getDir());
//            const Exit* nextExit = it->first.getFinish();
//            int length = it->first.getLength();
//
//            compTo->chooseSolution(&it->first);
//            level->Free -= length;
//            compTo->incrementOccupied(length);
//
//            if (compTo->getOccupiedCount() == compTo->getSize())
//            {
//                level->componentsFullyTraversed++;
//            }
//            
//            if (level->componentsFullyTraversed + 1 == level->getComponentCount())
//            {
//                // Almost done
//                printf("ALMOST DONE! In component %d\n", nextExit->getComponentId());
//            }
//
//            follow(nextExit);
//            
//            if (compTo->getOccupiedCount() == compTo->getSize())
//            {
//                level->componentsFullyTraversed--;
//            }
//
//            compTo->decrementOccupied(length);
//            level->Free += it->first.getLength();
//            compTo->unchooseSolution();
//        }
//    }
//}

void Solver::trySolving(int startX, int startY)
{
    Cell* cell = level->getCell(startY, startX);

    for (int d = 0; d < 4; d++)
    {
        /*if (cell->hasExit(d))
        {
            const Exit* exit = cell->getExit(d);
            follow(exit);
        }*/

        if (mayStartFrom(cell, d))
        {
            backtrack(cell, d);

            if (stopBacktracking())
            {
                return;
            }
        }
    }
}
