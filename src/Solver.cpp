#include "Common.h"
#include "Colorer.h"
#include "General.h"
#include "Obstacle.h"
#include "Component.h"
#include "Solver.h"

Solver::Solver(Level* currentLevel, const char* _outputFilename)
    : Simulator(currentLevel)
    , outputFilename(_outputFilename)
{
}

void Solver::solve(int row, int col, int firstComponentId)
{
    std::set<const Component*> specialComponents = level->getSpecialComponents();

    if (level->Ends == 2)
    {
        throw std::exception("Not implemented");
    }
    else if (level->Ends == 1)
    {
        throw std::exception("Not implemented");
    }

    if (specialComponents.size() == 2)
    {
        // We know the first and the last componentes
        throw std::exception("Not implemented");
    }
    else if (specialComponents.size() == 1)
    {

    }

    for (int startY = row; startY <= level->getHeight(); startY++)
    {
        for (int startX = col; startX <= level->getWidth(); startX++)
        {
            TRACE(printf("%d,%d\n", startX, startY));

            const Cell* cell = level->getCell(startY, startX);

            if (cell->isExit())
            {
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
                    level->setSolutionStartXY(startX, startY);
                    level->prepareSolution();
                    level->outputToFile(outputFilename);

                    return;
                }
            }
        }
    }
}

void Solver::follow(const Exit* exit)
{
//TODO: check if can actually move to that component
//TODO: check if final component (count fully-traversed components, if only one left untraversed, and current component has some free cells left - then we are trying to finish)
//TODO: think of accumulating answer (add sequence of Cell* to every Path, nay?)

//TODO: figure out how to start and how to terminate
    // Get opposing exit
    int dir = exit->getDir();
    const Exit* opposingExit = exit->getNextCell(dir)->getExit(dir ^ 2); // TODO: consider saving a pointer in Exit class

    if (opposingExit->isFree() == false)
    {
        return;
    }

    Component* compFrom = &level->getComponents()[exit->getComponentId()];
    Component* compTo = &level->getComponents()[opposingExit->getComponentId()];

    //if (compFrom->getSolutionCount() == 0)
    //{
    //    // Must be an 'end'
    //    if (level->Free == 1)
    //    {
    //        printf("LAST END!\n");
    //    }
    //    else
    //    {
    //        printf("FIRST END???\n");
    //    }
    //}
    //
    //if (compTo->getSolutionCount() == 0)
    //{
    //    // Must be an 'end'
    //    if (level->Free == 1)
    //    {
    //        printf("LAST END???\n");
    //    }
    //    else
    //    {
    //        printf("FIRST END!\n");
    //    }
    //}

    const SolutionMap* solutions = compTo->getSolutions();
    FOREACH(*solutions, it)
    {
        if (*it->first.getStart() == *opposingExit)
        {
            //printf("Following (%d, %d, %d)...\n", opposingExit->getX(), opposingExit->getY(), opposingExit->getDir());
            const Exit* nextExit = it->first.getFinish();
            int length = it->first.getLength();

            compTo->chooseSolution(&it->first);
            level->Free -= length;
            compTo->incrementOccupied(length);

            if (compTo->getOccupiedCount() == compTo->getSize())
            {
                level->componentsFullyTraversed++;
            }
            
            if (level->componentsFullyTraversed + 1 == level->getComponentCount())
            {
                // Almost done
                printf("ALMOST DONE! In component %d\n", nextExit->getComponentId());
            }

            follow(nextExit);
            
            if (compTo->getOccupiedCount() == compTo->getSize())
            {
                level->componentsFullyTraversed--;
            }

            compTo->decrementOccupied(length);
            level->Free += it->first.getLength();
            compTo->unchooseSolution();
        }
    }
}

void Solver::trySolving(int startX, int startY)
{
    Cell* cell = level->getCell(startY, startX);

    for (int d = 0; d < 4; d++)
    {
        if (cell->hasExit(d))
        {
            const Exit* exit = cell->getExit(d);
            follow(exit);
        }

        /*if (mayStartFrom(cell, d))
        {
            backtrack(cell, d);

            if (stopBacktracking())
            {
                return;
            }
        }*/
    }
}

// Looking for a solution. 'from' - our current direction
//void Solver::Solve(Cell* cell, int from)
//{
//    /*if (Debug::traceFlag && cell->getX() == 8 && cell->getY() == 2)
//    {
//        int alpha = 0733;
//    }*/
//
//    /*SolverDepth++;*/
//
//    if (level->Free == 0) // no free cells left
//    {
//        level->Solved = true;
//        return;
//    }
//
//    // Trying both orthogonal directions
//    for (int dir = !(from & 1); dir < 4; dir += 2)
//    {
//        if (!(cell->next & P[dir])) // An obstacle
//        {
//            continue;
//        }
//
//        int rdir = dir ^ 2; // opposite direction
//
//        // Remove bond from a neighbour 'from behind'
//        if (cell->next & P[rdir])
//        {
//            cell->Next[rdir]->next ^= P[dir];
//            if (cell->Next[rdir]->isPit())
//            {
//                level->Ends++;
//            }
//        }
//
//        Cell* ncell = cell->Next[dir]; // Second cell in the row
//
//        bool separated = false;
//        bool entranceIsSpecial = false;
//
//        int left = Left[dir];
//        int right = Right[dir];
//
//        int line = 1; // number of cells in the line
//
//        int nends = 0; // new 'ends' along the line
//
//        while (ncell->next & P[dir]) // Not considering last cell
//        {
//#ifdef TEMPORARY
//            if (ncell->getComponentId() != CurrentComponent)
//            {
//                CurrentComponent = ncell->getComponentId();
//                if (Components[CurrentComponent].Occupied() == 0 && ncell->mayBeFirst() == false)
//                {
//                    entranceIsSpecial = true;
//                }
//            }
//            Components[CurrentComponent].IncrementOccupied();
//#endif
//
//            ncell->setFree(false);
//
//            // Checking orthogonal directions
//            if (ncell->next & P[left])
//            {
//                ncell->Next[left]->next ^= P[right];
//                if (ncell->Next[left]->isPit())
//                {
//                    nends++;
//                }
//            }
//
//            if (ncell->next & P[right])
//            {
//                ncell->Next[right]->next ^= P[left];
//                if (ncell->Next[right]->isPit())
//                {
//                    nends++;
//                }
//            }
//
//            // Consider touching components
//            for (int t = 0; t < ncell->touch; t++) // At most 4 times (absolutely rarely)
//            {
//                Obstacle& c = level->getObstacles()[ncell->Touch[t]];
//                c.IncrementTouched();
//
//                if (c.Touched() > 1)
//                {
//                    // Is there any same-component-touching cell that has been visited before?
//                    bool ok = false;
//                    for (int k = 0; k < ncell->nexttouch[t]; k++)
//                    {
//                        if (!(ncell->NextTouch[t][k]->isFree())) // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! try to avoid using FREE/NOT FREE
//                        {
//                            ok = true;
//                            break;
//                        }
//                    }
//                    // If not then we have just separated some cells
//                    if (ok == false)
//                    {
//                        separated = true;
//                    }
//                }
//            }
//
//            ncell = ncell->Next[dir]; // Moving forward
//
//            line++;
//        }
//
//        // Consider last cell
//
//        ncell->setFree(false);
//
//        // Last cell: consider touching components
//        for (int t = 0; t < ncell->touch; t++)
//        {
//            Obstacle& c = level->getObstacles()[ncell->Touch[t]];
//            c.IncrementTouched();
//
//            if (c.Touched() > 1)
//            {
//                // Is there any same-component-touching cell that has been visited before?
//                bool ok = false;
//                for (int k = 0; k < ncell->nexttouch[t]; k++)
//                {
//                    if (!(ncell->NextTouch[t][k]->isFree())) // Try to avoid using FREE / UNFREE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//                    {
//                        ok = true;
//                        break;
//                    }
//                }
//                if (ok == false)
//                {
//                    separated = true;
//                }
//            }
//        }
//
//#ifdef TEMPORARY
//        if (ncell->getComponentId() != CurrentComponent)
//        {
//            CurrentComponent = ncell->getComponentId();
//            if (Components[CurrentComponent].Occupied() == 0 && ncell->mayBeFirst() == false)
//            {
//                entranceIsSpecial = true;
//            }
//        }
//        Components[CurrentComponent].IncrementOccupied();
//#endif
//
//#ifdef TEMPORARY
//        if (entranceIsSpecial && nends == 0)
//        {
//            // Assuming that special entrance adds at least one additional end
//            nends++;
//        }
//#endif
//
//        level->Free -= line;
//
//        if (level->Ends + nends <= 1)
//        {
//            if (!separated)
//            {
//                level->Ends += nends;
//
//                TRACE
//                (
//                    level->traceComponent();
//                    printf("tracer.depth = %d\n", tracer.depth);
//                    system("pause");
//                );
//
//                //ncell->setMark(1);
//                /*if (Debug::traceFlag)
//                {
//                    Component::Trace();
//                }*/
//
//                Solve(ncell, dir); // Recursing
//                //ncell->setMark(0);
//
//                if (level->Solved)
//                {
//                    /*TRACE
//                    (
//                        level->traceComponent();
//                        printf("tracer.depth = %d\n", tracer.depth);
//                        system("pause");
//                    );*/
//
//                    //level->Answer[level->ans++] = Direction[dir]; // Accumulating answer in reversed order
//                    return;
//                }
//                level->Ends -= nends;
//            }
//        }
//
//        // --------------------------------- Recover ---------------------------------
//        level->Free += line;
//
//        // Last cell
//#ifdef TEMPORARY
//        Components[CurrentComponent].DecrementOccupied();
//        if (ncell->getComponentId() != CurrentComponent)
//        {
//            CurrentComponent = ncell->getComponentId();
//        }
//#endif
//        ncell->setFree(true);
//        for (int t = 0; t < ncell->touch; t++)
//        {
//            Obstacle& c = level->getObstacles()[ncell->Touch[t]];
//            c.DecrementTouched();
//        }
//
//        ncell = ncell->Next[rdir];
//
//        while (ncell != cell) // All cells except for the first and the last ones
//        {
//#ifdef TEMPORARY
//            Components[CurrentComponent].DecrementOccupied();
//            if (ncell->getComponentId() != CurrentComponent)
//            {
//                CurrentComponent = ncell->getComponentId();
//            }
//#endif
//
//            if (ncell->next & P[left])
//            {
//                ncell->Next[left]->next ^= P[right];
//            }
//            if (ncell->next & P[right])
//            {
//                ncell->Next[right]->next ^= P[left];
//            }
//            ncell->setFree(true);
//            // Recovering touches
//            for (int t = 0; t < ncell->touch; t++)
//            {
//                Obstacle& c = level->getObstacles()[ncell->Touch[t]];
//                c.DecrementTouched();
//            }
//
//            ncell = ncell->Next[rdir];
//        }
//
//        // First cell: recover the bond from a neighbour 'from behind'
//        if (cell->next & P[rdir])
//        {
//            if (cell->Next[rdir]->isPit())
//            {
//                level->Ends--;
//            }
//
//            cell->Next[rdir]->next ^= P[dir];
//        }
//    }
//    //SolverDepth--;
//}

//void Solver::TrySolving(int startX, int startY)
//{
//    Cell* cell = level->getCell(startY, startX);
//
//    if (cell->isFree())
//    {
//        // Skipping 'through' cells (which have 2 neighbours with 2 neighbours)
//        if (cell->isThrough())
//        {
//            return;
//        }
//
//        if (cell->isPit())
//        {
//            level->Ends--;
//        }
//
//#ifdef TEMPORARY
//        if (cell->getComponentId() != CurrentComponent)
//        {
//            CurrentComponent = cell->getComponentId();
//        }
//        Components[CurrentComponent].IncrementOccupied();
//#endif
//        cell->setFree(false);
//        level->Free--;
//
//        UpdateTouchingObstacles(cell, true);
//
//        // Enter the cell in two different ways (due to optimization of Solve())
//        for (int d = 0; d <= 1; d++) // 0 - hor, 1 - ver
//        {
//            int nends = 0;
//            // Destroy bonds with neighbours
//            // Check for 'ends' along the direction
//            for (int dir = d; dir < 4; dir += 2)
//            {
//                if (cell->next & P[dir])
//                {
//                    cell->Next[dir]->next ^= P[dir ^ 2];
//
//                    if (cell->Next[dir]->isPit())
//                    {
//                        nends++;
//                    }
//                }
//            }
//
//            level->Ends += nends;
//
//            if (level->Ends <= 2)
//            {
//                tracer.currentComponent = cell->getComponentId();
//
//                //Component::Trace();
//                Solve(cell, d); // *** Calling main solving function ***
//                //Component::Trace(CurrentComponent);
//
//                tracer.currentComponent = -1;
//
//                if (level->Solved)
//                {
//                    level->setSolutionStartXY(startX, startY);
//                    break;
//                }
//            }
//
//            level->Ends -= nends;
//
//            // Recover neighbours
//            for (int dir = d; dir < 4; dir += 2)
//            {
//                if (cell->next & P[dir])
//                {
//                    cell->Next[dir]->next ^= P[dir ^ 2];
//                }
//            }
//        }
//
//        UpdateTouchingObstacles(cell, false);
//        /*for (int t = 0; t < cell->touch; t++)
//        {
//            level->getObstacles()[cell->Touch[t]].DecrementTouched();
//        }*/
//
//        cell->setFree(true);
//        level->Free++;
//
//#ifdef TEMPORARY
//        Components[CurrentComponent].DecrementOccupied();
//        if (cell->getComponentId() != CurrentComponent)
//        {
//            CurrentComponent = cell->getComponentId();
//        }
//#endif
//        if (cell->isPit())
//        {
//            level->Ends++;
//        }
//    }
//}


bool Solver::reachedFinalCell(Cell* cell, int dir) const
{
    // If we are at the very last remaining cell
    return level->Free == 1;
}

void Solver::solutionFound(Cell* cell, int dir)
{
    Colorer::print<WHITE>("Solution found!!!\n");
    level->Solved = true;
}
