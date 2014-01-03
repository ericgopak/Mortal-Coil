#include "Common.h"
#include "Colorer.h"
#include "General.h"
#include "Component.h"
#include "Analyzer.h"

Analyzer::Analyzer(Level* currentLevel)
    : Simulator(currentLevel)
    , componentCurrentIndex(-1)
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

            analyzeComponent(component);
            
            Colorer::print<WHITE>("Found %d solution%c\n", component.getSolutionCount(), component.getSolutionCount() == 1 ? ' ' : 's');

            if (component.getSolutionCount() == 1)
            {
                Colorer::print<YELLOW>("Component %d has ONLY ONE Solution!\n", i);
            }
            else if (component.getSolutionCount() == 0)
            {
                Colorer::print<YELLOW>("Component %d is SPECIAL!\n", getComponentCurrentIndex(), i);

                level->traceComponent(i);

                level->addSpecialComponent(&component, i);
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
            assert(false && "Not supposed to happen!");
        }
    }
    else
    {
        FOREACH(component.getExits(), it)
        {
            const Exit* e = *it;
            if (level->getCell(e->getY(), e->getX())->isOccupied())
            {
                continue;
            }

            int dir = e->getDir();
            
            Cell* cell = level->getCell(e->getY(), e->getX());
            
            int lastDepthCopy = prevDepth;
            prevDepth = depth;

            previousExit.push_back(e);
            backtrack(cell, dir ^ 2);
            previousExit.pop_back();

            prevDepth = lastDepthCopy;
        }
    }
}

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

    // Assign opposing exit to every existing one
    findOpposingExits();

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
    if (left->isTemporaryEnd() && left->getComponentId() == cell->getComponentId())
    {
        level->addTemporaryEnd(left);
    }
    if (right->isTemporaryEnd() && right->getComponentId() == cell->getComponentId())
    {
        level->addTemporaryEnd(right);
    }
}

void Analyzer::preRestoreAction(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    if (left->isTemporaryEnd() && left->getComponentId() == cell->getComponentId())
    {
        level->removeTemporaryEnd(left);
    }
    if (right->isTemporaryEnd() && right->getComponentId() == cell->getComponentId())
    {
        level->removeTemporaryEnd(right);
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
    if (level->getTemporaryEnds().size() > 2)
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

