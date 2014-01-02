#include "Common.h"
#include "Colorer.h"
#include "General.h"
#include "Component.h"
#include "Analyzer.h"

Analyzer::Analyzer(Level* currentLevel)
    : Simulator(currentLevel)
    , componentCurrentIndex(-1)
    /*, depth(0)
    , prevDepth(0)*/
{
}

int Analyzer::depth = 0;
int Analyzer::prevDepth = 0;

int Analyzer::getComponentCurrentIndex() const
{
    return componentCurrentIndex;
}

void Analyzer::setComponentCurrentIndex(int index)
{
    componentCurrentIndex = index;
}

void Analyzer::analyzeComponents()
{
    for (int i = 0; i < level->getComponentCount(); i++)
    {
        Component& component = level->getComponents()[i];

        if (component.getSize() == 1)
        {
            const std::set<const Exit*>& exits = component.getExits();

            if (component.getExits().size() == 2)
            {
                const Exit* e1 = *exits.begin();
                const Exit* e2 = *(++exits.begin());

                SolutionMap solution1;
                SolutionMap solution2;

                Path p1(e1, e2, 1);
                Path p2(e2, e1, 1);

                solution1[p1] = ComponentSolution();
                solution2[p2] = ComponentSolution();

                component.addSolution(&solution1);
                component.addSolution(&solution2);
            }
            else if (component.getExits().size() == 1)
            {
                const Exit* e = *exits.begin();
                TRACE(Colorer::print<YELLOW>("Double check: end at (%d, %d)  (one exit only)\n", e->getX(), e->getY()));
            }
        }
        else if (component.getSize() > 1)
        {
            setComponentCurrentIndex(i);

            level->traceComponent(i);
            printf("CONSIDERING %d (%d)\n", i, component.getSize());

            Analyzer::analyzeComponent(component);

            Colorer::print<WHITE>("Found %d solutions\n", component.getSolutionCount());

            if (component.getSolutionCount() == 1)
            {
                Colorer::print<YELLOW>("Component %d has ONLY ONE Solution!\n", i);
            }
            else if (component.getSolutionCount() == 0)
            {
                Colorer::print<YELLOW>("Component %d is SPECIAL!\n", getComponentCurrentIndex(), i);

                level->traceComponent(i);

                level->addSpecialComponent(&component);

                /*FOREACH_CONST(comp.getExits(), e)
                {
                    level->getCell((*e)->getY(), (*e)->getX())->SetMayBeFirst();
                }*/
            }
        }
    }
}

void Analyzer::analyzeComponent(Component& component)
{
    tracer.currentX = -1;
    tracer.currentY = -1;
    //tracer.currentDir = -1;

    /*SolutionMap solution;
    solutionIterator = &solution;*/

    if (component.getSize() == 0)
    {
        if (solutionHolder.size() > 0)
        {
            /*Colorer::print<WHITE>("Oh yeah! Found full solution!  ");

            for (size_t i = 0; i < solutionHolder.size(); i++)
            {
                const Exit* a = solutionHolder[i].getStart();
                const Exit* b = solutionHolder[i].getFinish();
                Colorer::print<WHITE>("(%d,%d,%d) --> (%d,%d,%d)  "
                    , a->getX(), a->getY(), a->getDir()
                    , b->getX(), b->getY(), b->getDir());
            }

            printf("\n");*/

            SolutionMap solution;
            SolutionMap* solutionFollower = &solution;

            for (size_t i = 0; i < solutionHolder.size(); i++)
            {
                (*solutionFollower)[solutionHolder[i]] = ComponentSolution();
                solutionFollower = solutionFollower->begin()->second.getSolutions();
            }

            component.addSolution(&solution);
        }
        else
        {
            throw new std::exception("Not supposed to happen!");
        }
        //level->traceComponent(getComponentCurrentIndex());

        //Analyzed = true;
        /*
        component.addSolution(solution); // TODO: populate 'solution'
        */
        
        //level->getCell(tracer.currentY, tracer.currentX)->SetMayBeFirst();

        /*Component::Trace();
        printf("\n");*/
        //system("pause");
    }
    else
    {
        //tracer.depth++;
        FOREACH(component.getExits(), it)
        {
            const Exit* e = *it;
            if (level->getCell(e->getY(), e->getX())->isOccupied())
            {
                continue;
            }

            //for (int dir = 0; dir < 4; dir++)
            //{
            int dir = e->getDir();
                //if (e->getDir() == dir)
                //{
            //printf("Exit: (%d %d -> %d)\n", (*e)->getX(), (*e)->getY(), dir);
            //if (tracer.depth == 1)
            //{
            //    tracer.currentX = e->getX();
            //    tracer.currentY = e->getY();
            //    //tracer.currentDir = dir ^ 2;

            //    level->Analyzed = false;
            //}

            //if (!level->Analyzed)
            //{
            Cell* cell = level->getCell(e->getY(), e->getX());
            //previousExit = e;

            int lastDepthCopy = prevDepth;
            prevDepth = depth;

            previousExit.push_back(e);
            backtrack(cell, dir ^ 2);
            previousExit.pop_back();

            prevDepth = lastDepthCopy;
            //}
                //}
            //}
        }
        //tracer.depth--;
    }
}

//void Analyzer::analyzeSolve(Cell* cell, int from)
//{
//    //Component::Trace(cell->getComponentId());
//
//    if (level->Analyzed)
//    {
//        return;
//    }
//
//    static int depth = 0;
//    depth++;
//
///*#ifdef DEBUG
//    printf("AnalyzeSolve:\n");
//    printf("%d %d  %d  from %d\n", cell->getX(), cell->getY(), cell->getComponentId(), from);
//#endif*/
//
//
//    //if (level->getComponents()[current_component].Size() == 0) // no free cells left
//    //{
//    //    Analyzed = true;
//    //    printf("Found a possible solution for component[%d]\n", current_component);
//    //    return;
//    //}
//
//    // Trying both orthogonal directions
//    for (int dir = !(from & 1); dir < 4; dir += 2)
//    {
//        //printf("Dir = %d  depth=%d\n", dir, depth);
//
//        //if (!(cell->next & P[dir]))
//        if (cell->Next[dir]->isObstacle())
//        {
//            //printf("Obstacle at (%d,%d)\n", cell->Next[dir]->getX(), cell->Next[dir]->getY());
//            continue; // An obstacle, better luck next time
//        }
//        // TODO: This is WORKAROUND due to BUGS
//        if (cell->Next[dir]->isOccupied())
//            continue;
//
//        // Don't allow to leave the component
//        if (cell->Next[dir]->getComponentId() != level->componentCurrentIndex)
//        {
//            continue;
//        }
//
//        // Reconnect with previously detached neighbour
//        Reconnect(cell, dir);
//
//        Cell* ncell = cell->Next[dir]; // Second cell in the row
//        Cell* exit = NULL;
//
//        unsigned int flags = 0;
//
//        //while (ncell->next & P[dir]) // Not considering last cell
//        while (ncell->Next[dir]->isFree() && ncell->Next[dir]->getComponentId() == level->componentCurrentIndex)
//        {
//            Simulator::MoveForward(ncell, dir);
//        }
//
//        // Process last cell
//        //if (ncell->isFree())
//        //{
//            //flags |= Occupy(ncell, dir);
//            Occupy(ncell, dir);
//            /*if (ncell->is_exit)
//            {
//                exit = ncell;
//            }*/
//        //}
//
//        // ------------------------ Goind deeper --------------------------------
////#ifdef DEBUG
////        printf("%%%%%%%%%% Component[%d].Size() = %d\n", current_component, level->getComponents()[current_component].Size());
////        Component::Trace();
//
////        int cnt = 0;
////        for (int i = 1; i <= H; i++) for (int j = 1; j <= W; j++) if (Grid[i][j].getComponentId() == current_component && Grid[i][j].free) cnt++;
////        if (cnt != level->getComponents()[current_component].Size())
////        {
////            printf("WOOOOOOOOOOOOOOOOOOOOOOOOOW!\n");
////            printf("cnt=%d vs size=%d\n", cnt, level->getComponents()[current_component].Size());
////            system("pause");
////        }
//
////        //system("pause");
////#endif
//        if (flags & SEPARATED)
//        {
//            printf("Got separated... stopped at (%d, %d)\n", ncell->getX(), ncell->getY());
//        }
//        /*else if (Ends > 2)
//        {
//            printf("Too many ends: %d\n", Ends);
//        }*/
//        else if (ncell->is_exit)
//        {
//            //printf("Good! Now we can go deeper!\n");
//
//            //if (!(ncell->next & P[dir])) // Obstacle ahead
//            //if (ncell->Next[dir]->isObstacle()) // Obstacle ahead
//            {
//                //printf("STAYING IN THE COMPONENT\n");
//                // Try staying in the component
//                analyzeSolve(ncell, dir);
//            }
//            // Suppose we have gone out of the component
//            analyzeComponent(level->getComponents()[level->componentCurrentIndex]);
//        }
//        else
//        {
//            analyzeSolve(ncell, dir);
//
//            //if (Analyzed)
//            //{
//            //    //Answer[ans++] = Direction[dir]; // Accumulating answer in reversed order
//            //    printf("Analyzed!\n");
//            //    return;
//            //}
//        }
//
//        // --------------------------------- Recover ---------------------------------
//
//        //printf("Recovering... (%d, %d)\n", ncell->getX(), ncell->getY());
//        while (ncell != cell) // All cells except for the first and the last ones
//        {
//            MoveBackwards(ncell, dir);
//            //level->getComponents()[current_component].Size()++;
//        }
//        //printf("Done recovering... (%d, %d)\n", ncell->getX(), ncell->getY());
//
//        // First cell: recover the bond with a neighbour 'from behind'
//        // Disconnect with previously reconnected neighbour
//        Disconnect(cell, dir);
//    }
//    depth--;
//}
//
//void Analyzer::analyze(int startX, int startY, int dir)
//{
//    if (level->Analyzed)
//    {
//        return;
//    }
///*#ifdef DEBUG
//    printf("Analyze(%d, %d, %d)\n", startX, startY, dir);
//#endif*/
//    Cell* cell = level->getCell(startY, startX);
//
//    if (cell->isFree()) // If is a free square
//    {
//        if (cell->isPit()) // Starting from a pit
//        {
//            level->Ends--;
//        }
//        // Seems to suck because a new pit may have appeared // TODO:
//        // Going straight
//        if (!(cell->next & P[dir])) // An obstacle right in front
//        {
//            Disconnect(cell, dir ^ 2);
//            Occupy(cell, dir);
//
//            analyzeSolve(cell, dir);
//
//            Restore(cell, dir);
//            Reconnect(cell, dir ^ 2);
//        }
//        else
//        {
//            // Remove bond from the neighbour 'from behind'
//            Disconnect(cell, dir ^ 2);
//
//            Cell* exit = NULL;
//
//            Cell* ncell = cell;
//
//            unsigned int flags = 0;
//
//            while (ncell->Next[dir]->isFree() && ncell->Next[dir]->getComponentId() == level->componentCurrentIndex)
//            {
//                MoveForward(ncell, dir);
//            }
//
//            Occupy(ncell, dir);
//
//            // GOING DEEPER ----------------------------------------------------------------------------------------------------------------
////#ifdef DEBUG
////            printf("$$$$$$$$$$ Component[%d].Size() = %d\n", current_component, level->getComponents()[current_component].Size());
////            Component::Trace();
//
////            int cnt = 0;
////            for (int i = 1; i <= H; i++) for (int j = 1; j <= W; j++) if (Grid[i][j].getComponentId() == current_component && Grid[i][j].free) cnt++;
////            if (cnt != level->getComponents()[current_component].Size())
////            {
////                printf("WOOOOOOOOOOOOOOOOOOOOOOOOOW!\n");
////                printf("cnt=%d vs size=%d\n", cnt, level->getComponents()[current_component].Size());
////                system("pause");
////            }
//
////            //system("pause");
////#endif
//            if (flags & SEPARATED)
//            {
//                printf("Got separated... stopped at (%d, %d)\n", ncell->getX(), ncell->getY());
//            }
//            /*else if (Ends > 2)
//            {
//                printf("Too many ends: %d\n", Ends);
//            }*/
//            else if (ncell->is_exit)
//            {
//                if (ncell->Next[dir]->isObstacle()) // Obstacle ahead
//                {
//                    // Try staying in the component
//                    analyzeSolve(ncell, dir);
//                }
//                // Suppose we have gone out of the component
//                analyzeComponent(level->getComponents()[level->componentCurrentIndex]);
//            }
//            else
//            {
//                if (!level->Analyzed)
//                {
//                    analyzeSolve(ncell, dir);
//                }
//            }
//            // RECOVERING --------------------------------------------------------------------------------------------------------------------
//
//            while (ncell != cell) // All cells except for the first and the last ones
//            {
//                MoveBackwards(ncell, dir);
//            }
//
//            Restore(cell, dir);
//
//            // First cell: recover the bond with a neighbour 'from behind'
//            Reconnect(cell, dir ^ 2);
//        }
//
//        if (cell->isPit())
//        {
//            level->Ends++; // Freeing up this pit
//        }
//    }
//}

// Create components, preprocess them

void Analyzer::preprocess()
{
    // Calculate mask of free neighbours for each cell
    // Counting total number of Edges
    findNeighbours();

    // Create 'bonds' between every two neighbours
    createBonds();

    // Check for 'ends' beforehand
    countEnds();

    // Split obstacles into components with Flood-fill
    findObstacles();

    // Note: must be after createBonds
    // TODO: move out creation of Exit's
    findComponents();

    TRACE
    (
        level->traceComponent();
    );

    // Analyzing obstacles ------------------------------------------------------------
    // For every free cell: list obstacles that it touches
    findTouchingObstacles();

    // For every free cell: list neighbouring free cell touching the same obstacle
    findFreeCellsTouchingSameObstacles();

    // Find biggest component
    int bestid = 0;
    int best = level->getComponents()[0].getSize();
    for (int i = 0; i < level->getComponentCount(); i++)
    {
        if (level->getComponents()[i].getSize() > best)
        {
            best = level->getComponents()[i].getSize();
            bestid = i;
        }
    }

    TRACE
    (
        Colorer::print<WHITE>("Biggest component: %d cells  ID: %d\n", best, bestid);
    );

    level->setMostCells(best);
    level->setBiggest(bestid);
    //Component::Trace(Component::Biggest);
    //Component::Trace();

#ifdef TRACE_STATISTICS
    std::map<int, int> stats;
    for (int i = 0; i < Component::Count; i++)
    {
        int sols = level->getComponents()[i].Solutions();
        if (!stats[sols])
        {
            stats[sols] = 1;
        }
        else
        {
            stats[sols]++;
        }
    }

    FOREACH(stats, it)
    {
        printf("Stats: %d components --> %d solutions\n", it->second, it->first);
    }
#endif
}

void Analyzer::preAction(Cell* cell, int dir) const
{
    depth++;
}

void Analyzer::postAction(Cell* cell, int dir) const
{
    depth--;
}

void Analyzer::preOccupyAction(Cell* cell, int dir) const
{
    // Note: touching obstacles does not really work
    // because we do not simulate the whole solution
    // (only partial)

    cell->setMark(tracer.depth);
}

void Analyzer::postOccupyAction(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    if (left->isEnd() && left->getComponentId() == cell->getComponentId())
    {
        level->Ends++;
    }
    if (right->isEnd() && right->getComponentId() == cell->getComponentId())
    {
        level->Ends++;
    }
}

void Analyzer::preRestoreAction(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    if (left->isEnd() && left->getComponentId() == cell->getComponentId())
    {
        level->Ends--;
    }
    if (right->isEnd() && right->getComponentId() == cell->getComponentId())
    {
        level->Ends--;
    }
}

void Analyzer::postRestoreAction(Cell* cell, int dir) const
{
    cell->setMark(0);
}

bool Analyzer::reachedFinalCell(Cell* cell, int dir) const
{
    int t = cell->getComponentId();
    if (cell->isExit())
    {
        if (cell->getNextCell(dir)->isObstacle())
        {
            // Try left / right
            int leftDirection = Left[dir];
            if (cell->hasExit(leftDirection))
            {
                if (cell->getNextCell(leftDirection)->isFree())
                {
                    return true;
                }
            }
            
            int rightDirection = Right[dir];
            if (cell->hasExit(rightDirection))
            {
                if (cell->getNextCell(rightDirection)->isFree())
                {
                    return true;
                }
            }
        }
        else
        {
            if (cell->hasExit(dir))
            {
                if (cell->getNextCell(dir)->isFree())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Analyzer::potentialSolution(Cell* cell, int dir) const
{
    if (level->Ends > 2)
    {
        return false;
    }

    return true;
}

void Analyzer::solutionFound(Cell* cell, int dir)
{
    // Assuming cell->isExit()
    
    //level->traceComponent(cell->getComponentId());

    if (cell->getNextCell(dir)->isObstacle())
    {
        // Try left / right
        int leftDirection = Left[dir];
        if (cell->getNextCell(leftDirection)->isFree())
        {
            if (cell->hasExit(leftDirection))
            {
                Path path(previousExit.back(), cell->getExit(leftDirection), depth - prevDepth);
                solutionHolder.push_back(path);
                occupy(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()]);
                restore(cell, dir);
                solutionHolder.pop_back();
            }
            else
            {
                // continue: turn left
                backtrack(cell, leftDirection);
            }
        }
            
        int rightDirection = Right[dir];
        if (cell->getNextCell(rightDirection)->isFree())
        {
            if (cell->hasExit(rightDirection))
            {
                Path path(previousExit.back(), cell->getExit(rightDirection), depth - prevDepth);
                solutionHolder.push_back(path);
                occupy(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()]);
                restore(cell, dir);
                solutionHolder.pop_back();
            }
            else
            {
                // continue: turn right
//                level->traceComponent(getComponentCurrentIndex());
                backtrack(cell, rightDirection);
            }
        }
    }
    else
    {
        if (cell->hasExit(dir))
        {
            if (cell->getNextCell(dir)->isFree())
            {
                Path path(previousExit.back(), cell->getExit(dir), depth - prevDepth);
                solutionHolder.push_back(path);
                occupy(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()]);
                restore(cell, dir);
                solutionHolder.pop_back();
            }

            // TODO: there's a chance that the cell in front is occupied
            // Try turning left/right with _precondition_
            {

            }
        }
    }
}
