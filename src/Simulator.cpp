#include "Simulator.h"
#include "Colorer.h"
#include "Common.h"
#include "Component.h"

Simulator::Simulator(Level* currentLevel)
    : level(currentLevel)
{
}

Simulator::~Simulator()
{
    //delete tracer;
}

//int Simulator::getComponentCurrentIndex() const
//{
//    return componentCurrentIndex;
//}
//
//int Simulator::getComponentCount() const
//{
//    return componentCount;
//}

//void Simulator::SetMostCells(int index)
//{
//    componentMostCellsIndex = index;
//}

//void Simulator::SetBiggest(int index)
//{
//    componentBiggestIndex = index;
//}

Simulator::TraceInfo::TraceInfo()
    : depth(0)
    , currentComponent(-1)
    , currentX(-1)
    , currentY(-1)
//    , currentDir(-1)
{
}

//unsigned int Simulator::Occupy(Cell*& cell, int dir)
//{
//    unsigned int r = 0;
//
//    cell->setFree(false);
//    level->Free--;
//    level->getComponents()[level->getComponentCurrentIndex()].DecrementSize();
//
//    TRACE(cell->setMark(tracer.depth + 1));
//
//    // Break bonds with neighbours
//    /*Disconnect(cell, Left[dir]);
//    Disconnect(cell, Right[dir]);*/
//    if (cell->Next[Left[dir]]->isFree()) Disconnect(cell, Left[dir]);
//    if (cell->Next[Right[dir]]->isFree()) Disconnect(cell, Right[dir]);
//
//    // Consider touching components
//    r |= CheckTouches(cell);
//
//    return r;
//}

void Simulator::occupy(Cell* cell, int dir) const
{
#ifdef DEBUG
    if (cell->isFree() == false)
    {
        throw std::exception("Occupying already non-free cell!");
    }
#endif

    cell->setFree(false);
    level->Free--;
    level->getComponents()[cell->getComponentId()].decrementSize();

    // Break bonds with neighbours
    /*Disconnect(cell, Left[dir]);
    Disconnect(cell, Right[dir]);*/
    //if (cell->Next[Left[dir]]->isFree()) Disconnect(cell, Left[dir]);
    //if (cell->Next[Right[dir]]->isFree()) Disconnect(cell, Right[dir]);

    // Consider touching components
    //r |= CheckTouches(cell);

    //return r;
}

//void Simulator::Restore(Cell*& cell, int dir)
//{
//    cell->setFree(true);
//    level->Free++;
//    level->getComponents()[level->getComponentCurrentIndex()].IncrementSize();
//
//    TRACE(cell->setMark(0));
//
//    // Recover bounds with neighbours
//    /*Reconnect(cell, Left[dir]);
//    Reconnect(cell, Right[dir]);*/
//    if (cell->Next[Left[dir]]->isFree()) Reconnect(cell, Left[dir]);
//    if (cell->Next[Right[dir]]->isFree()) Reconnect(cell, Right[dir]);
//
//    // Recovering touches
//    UncheckTouches(cell);
//}
void Simulator::restore(Cell* cell, int dir) const
{
    cell->setFree(true);
    level->Free++;
    level->getComponents()[cell->getComponentId()].incrementSize(); // TODO: decrement 'occupied' instead

    //cell->setMark(0);

    // Recover bounds with neighbours
    /*Reconnect(cell, Left[dir]);
    Reconnect(cell, Right[dir]);*/
    /*if (cell->Next[Left[dir]]->isFree()) Reconnect(cell, Left[dir]);
    if (cell->Next[Right[dir]]->isFree()) Reconnect(cell, Right[dir]);*/

    // Recovering touches
    //UncheckTouches(cell);
}

//unsigned int Simulator::MoveForward(Cell*& cell, int dir)
//{
//    unsigned int r = Occupy(cell, dir);
//
//    cell = cell->Next[dir];
//
//    return r;
//}
//
//void Simulator::MoveBackwards(Cell*& cell, int dir)
//{
//    Restore(cell, dir);
//
//    cell = cell->Next[dir ^ 2];
//}

Cell* Simulator::moveForward(Cell* cell, int dir) const
{
    preOccupyAction(cell, dir);
    occupy(cell, dir);
    postOccupyAction(cell, dir);

    return cell->getNextCell(dir);
}

Cell* Simulator::moveBackwards(Cell* cell, int dir) const
{
    cell = cell->getNextCell(dir ^ 2);

    preRestoreAction(cell, dir);
    restore(cell, dir);
    postRestoreAction(cell, dir);

    return cell;
}

void Simulator::findComponents() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false && level->getCell(i, j)->getComponentId() == -1)
            {
                int index = level->getComponentCount();
                
                level->getComponents().push_back(Component());

                floodComponent(j, i, index);
            }
        }
    }
}

void Simulator::findTouchingObstacles() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            Cell& c1 = *(level->getCell(i, j));
            if (c1.isObstacle() == false)
            {
                for (int dir = 0; dir < 8; dir++)
                {
                    Cell &c2 = *(level->getCell(i + dy[dir], j + dx[dir]));
                    if (c2.isObstacle())
                    {
                        bool is_new = true;
                        int oind = c2.getObstacleId();
                        for (int k = 0; k < c1.touch; k++)
                        {
                            if (oind == c1.Touch[k])
                            {
                                is_new = false;
                                break;
                            }
                        }
                        if (is_new)
                        {
                            c1.Touch[c1.touch++] = oind;
                        }
                    }
                }
            }
        }
    }
}

void Simulator::findFreeCellsTouchingSameObstacles() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            Cell &c1 = *(level->getCell(i, j));
            if (c1.isObstacle() == false)
            {
                for (int dir = 0; dir < 4; dir++)
                {
                    Cell &c2 = *(level->getCell(i + dy[dir], j + dx[dir]));
                    if (c2.isObstacle() == false)
                    {
                        for (int t1 = 0; t1 < c1.touch; t1++)
                        {
                            for (int t2 = 0; t2 < c2.touch; t2++)
                            {
                                if (c1.Touch[t1] == c2.Touch[t2])
                                {
                                    c1.NextTouch[t1][c1.nexttouch[t1]++] = &c2;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Simulator::findObstacles() const
{
    floodObstacle(0, 0, 0); // Special component - the one that includes borders
    level->getObstacles().push_back(Obstacle()); // TODO: call proper cons
    
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() && level->getCell(i, j)->getObstacleId() == -1)
            {
                int index = level->getObstacleCount();
                level->getObstacles().push_back(Obstacle()); // TODO: call proper cons
                // New obstacle found -> mark all the cells in it
                floodObstacle(j, i, index);
            }
        }
    }
}

void Simulator::floodObstacle(int x, int y, int num) const
{
    level->getCell(y, x)->setObstacleId(num);

    for (int dir = 0; dir < 8; dir++) // Diagonal directions as well
    {
        int ny = y + dy[dir];
        int nx = x + dx[dir];

        if (0 <= ny && ny <= level->getHeight() + 1)
        if (0 <= nx && nx <= level->getWidth() + 1)
        if (level->getCell(ny, nx)->isObstacle())
        if (level->getCell(ny, nx)->getObstacleId() == -1) // Not assigned
        {
            floodObstacle(nx, ny, num);
        }
    }
}

void Simulator::updateTouchingObstacles(Cell* cell, bool inc) const
{
    for (int t = 0; t < cell->touch; t++)
    {
        if (inc)
        {
            level->getObstacles()[cell->Touch[t]].IncrementTouched();
        }
        else
        {
            level->getObstacles()[cell->Touch[t]].DecrementTouched();
        }
    }
}

void Simulator::findNeighbours() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false)
            {
                for (int dir = 0; dir < 4; dir++)
                {
                    if (level->getCell(i + dy[dir], j + dx[dir])->isObstacle() == false)
                    {
                        level->getCell(i, j)->setNextMask(level->getCell(i, j)->getNextMask() | P[dir]);
                    }
                }
            }
        }
    }
}

void Simulator::createBonds() const
{
    for (int i = 0; i <= level->getHeight() + 1; i++)
    {
        for (int j = 0; j <= level->getWidth() + 1; j++)
        {
            for (int dir = 0; dir < 4; dir++)
            {
                if (0 <= i + dy[dir] && i + dy[dir] <= level->getHeight() + 1)
                if (0 <= j + dx[dir] && j + dx[dir] <= level->getWidth() + 1)
                {
                    printf("Setting nextCell for (%d,%d,%d)\n", i, j, dir);
                    level->getCell(i, j)->setNextCell(dir, level->getCell(i + dy[dir], j + dx[dir]));
                }
            }
        }
    }
}

void Simulator::countEnds() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false && level->getCell(i, j)->isPit())
            {
                level->EndX[level->Ends] = j;
                level->EndY[level->Ends] = i;
                level->Ends++;
            }
        }
    }
}

//void Simulator::connectOppositeExits() const
//{
//    for (int i = 1; i <= level->getHeight(); i++)
//    {
//        for (int j = 1; j <= level->getWidth(); j++)
//        {
//            Cell* cell = level->getCell(i, j);
//
//            // Connect opposing exits. Once.
//            if (cell->isExit())
//            {
//                for (int dir = 0; dir < 4; dir++)
//                {
//                    int ny = cell->getY() + dy[dir];
//                    int nx = cell->getX() + dx[dir];
//
//                    Cell* ncell = level->getCell(ny, nx);
//
//                    if (ncell->isExit())
//                    {
//                        cell->getExit();...
//                    }
//                }
//            }
//        }
//    }
//}

void Simulator::floodComponent(int x, int y, int num) const
{
    int mask = 0;

    Cell* cell = level->getCell(y, x);

    cell->setComponentId(num);
    level->getComponents()[num].incrementSize();

    for (int dir = 0; dir < 4; dir++)
    {
        int ny = y + dy[dir];
        int nx = x + dx[dir];

        if (1 <= ny && ny <= level->getHeight())
        {
            if (1 <= nx && nx <= level->getWidth())
            {
                Cell* ncell = level->getCell(ny, nx);

                if (ncell->isObstacle() == false)
                {
                    bool l = level->getCell(y + dy[Left[dir]], x + dx[Left[dir]])->isObstacle()
                        || level->getCell(ny + dy[Left[dir]], nx + dx[Left[dir]])->isObstacle();
                    bool r = level->getCell(y + dy[Right[dir]], x + dx[Right[dir]])->isObstacle()
                        || level->getCell(ny + dy[Right[dir]], nx + dx[Right[dir]])->isObstacle();

                    if (!(l && r))
                    {
                        if (ncell->getComponentId() == -1)
                        {
                            floodComponent(nx, ny, num);
                        }
                    }
                    else // is exit
                    {
                        Exit exit(cell, dir);
                        cell->addExit(exit);
                    }
                }
            }
        }
    }

    if (cell->isExit())
    {
        //level->getCell(y, x)->is_exit = true;

        FOREACH(cell->getExits(), e)
        {
            level->getComponents()[num].addExit(&*e);
            printf("Adding (%d,%d,%d) to component %d\n", e->getX(), e->getY(), e->getDir(), num);
        }
    }
}

void Simulator::touchObstacles(Cell* cell) const
{
    for (int t = 0; t < cell->touch; t++)
    {
        Obstacle& c = level->getObstacles()[cell->Touch[t]];
        c.IncrementTouched();
    }
}

bool Simulator::checkTouchingObstacles(Cell* cell) const
{
    for (int t = 0; t < cell->touch; t++)
    {
        Obstacle& o = level->getObstacles()[cell->Touch[t]];

        if (o.Touched() > 1) // Is there any same-component-touching cell that has been visited before?
        {
            bool ok = false;
            for (int k = 0; k < cell->nexttouch[t]; k++)
            {
                // Check if any 'neighbour' cells touched the same component
                if (!(cell->NextTouch[t][k]->isFree())) // Try to avoid using FREE / UNFREE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                {
                    ok = true;
                    break;
                }
            }
            if (!ok)
            {
                return false;
            }
        }
    }

    return true;
}

void Simulator::untouchObstacles(Cell* cell) const
{
    for (int t = 0; t < cell->touch; t++)
    {
        Obstacle& c = level->getObstacles()[cell->Touch[t]];
        c.DecrementTouched();
    }
}

void Simulator::preAction(Cell* cell, int dir) const
{
}

void Simulator::postAction(Cell* cell, int dir) const
{
    if (level->Solved)
    {
        // Accumulate answer
        level->prependSolutionCell(cell, dir);
    }
}

void Simulator::preOccupyAction(Cell* cell, int dir) const
{
    touchObstacles(cell);
}

void Simulator::postOccupyAction(Cell* cell, int dir) const
{
}

void Simulator::preRestoreAction(Cell* cell, int dir) const
{
}

void Simulator::postRestoreAction(Cell* cell, int dir) const
{
    untouchObstacles(cell);
}

bool Simulator::potentialSolution(Cell* cell, int dir) const
{
    if (checkTouchingObstacles(cell) == false)
    {
        return false;
    }

    return true;
}

bool Simulator::mayStartFrom(Cell* cell, int dir) const
{
    if (cell->isThrough()) return false;
    return true;
}

bool Simulator::shouldConsider(Cell* cell, int dir) const
{
    if (!cell->isFree())
    {
        return false;
    }
    return true;
}

bool Simulator::stopBacktracking() const
{
    return level->Solved;
}

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
//                    level->Answer[level->ans++] = Direction[dir]; // Accumulating answer in reversed order
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

//void Simulator::backtrack(Cell* cell, int direction, Simulator* engine)
//{
//    tracer.depth++;
//
//    if (tracer.depth >= 146)
//    {
//        level->traceComponent();
//    }
//
//    if (!engine->shouldConsider(cell, direction))
//    {
//        return;
//    }
//
//    if (cell->Next[direction]->isFree()) // Forward
//    {
//        cell = moveForward(cell, direction);
//
//if (Debug::traceFlag) level->traceComponent();
//
//        if (engine->potentialSolution(cell, direction))
//        {
//            if (engine->reachedFinalCell(cell, direction))
//            {
//                engine->solutionFound(cell, direction);
//            }
//            else
//            {
//                backtrack(cell, direction, engine);
//            }
//        }
//
//        cell = moveBackwards(cell, direction);
//    }
//    else
//    {
//        // Turn left
//        int leftDirection = Left[direction];
//        if (cell->Next[leftDirection]->isFree())
//        {
//            cell = moveForward(cell, leftDirection);
//if (Debug::traceFlag) level->traceComponent();
//            if (engine->potentialSolution(cell, leftDirection))
//            {
//                if (engine->reachedFinalCell(cell, leftDirection))
//                {
//                    engine->solutionFound(cell, leftDirection);
//                }
//                else
//                {
//                    backtrack(cell, leftDirection, engine);
//                }
//            }
//
//            cell = moveBackwards(cell, leftDirection);
//        }
//        
//        // Turn right
//        int rightDirection = Right[direction];
//        if (cell->Next[rightDirection]->isFree())
//        {
//            cell = moveForward(cell, rightDirection);
//if (Debug::traceFlag) level->traceComponent();
//            if (engine->potentialSolution(cell, rightDirection))
//            {
//                if (engine->reachedFinalCell(cell, rightDirection))
//                {
//                    engine->solutionFound(cell, rightDirection);
//                }
//                else
//                {
//                    backtrack(cell, rightDirection, engine);
//                }
//            }
//
//            cell = moveBackwards(cell, rightDirection);
//        }
//    }
//
//    tracer.depth--;
//}


void Simulator::trySolving(int startX, int startY)
{
    Cell* cell = level->getCell(startY, startX);

    for (int d = 0; d < 4; d++)
    {
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

void Simulator::backtrack(Cell* cell, int direction)
{
    if (!shouldConsider(cell, direction))
    {
        return;
    }

    TRACE(tracer.depth++);

    preAction(cell, direction);

    if (potentialSolution(cell, direction))
    {
        if (reachedFinalCell(cell, direction))
        {
            solutionFound(cell, direction);
        }
        else
        {
            if (cell->getNextCell(direction)->isFree()) // Forward
            {
                cell = moveForward(cell, direction);
                backtrack(cell, direction);
                cell = moveBackwards(cell, direction);
            }
            else
            {
                // Turn left
                int leftDirection = Left[direction];
                if (cell->getNextCell(leftDirection)->isFree())
                {
                    cell = moveForward(cell, leftDirection);
                    backtrack(cell, leftDirection);
                    cell = moveBackwards(cell, leftDirection);
                }

                if (stopBacktracking() == false)
                {
                    // Turn right
                    int rightDirection = Right[direction];
                    if (cell->getNextCell(rightDirection)->isFree())
                    {
                        cell = moveForward(cell, rightDirection);
                        backtrack(cell, rightDirection);
                        cell = moveBackwards(cell, rightDirection);
                    }
                }
            }
        }
    }

    postAction(cell, direction);

    TRACE(tracer.depth--);
}
