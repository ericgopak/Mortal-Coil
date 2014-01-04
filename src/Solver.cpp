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
    touchObstacles(cell);
}

void Solver::postOccupyAction(Cell* cell, int dir) const
{
}

void Solver::preRestoreAction(Cell* cell, int dir) const
{
}

void Solver::postRestoreAction(Cell* cell, int dir) const
{
    untouchObstacles(cell);
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
    if (checkTouchingObstacles(cell) == false)
    {
        return false;
    }

    return true;
}

void Solver::solutionFound(Cell* cell, int dir)
{
    TRACE(Colorer::print<WHITE>("Solution found!!!\n"));
    level->Solved = true;
}

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
