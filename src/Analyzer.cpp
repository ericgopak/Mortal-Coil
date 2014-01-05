#include "Common.h"
#include "Colorer.h"
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

                solution1[INITIAL_STATE_MASK][p1] = ComponentSolution();
                solution2[INITIAL_STATE_MASK][p2] = ComponentSolution();

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

            TRACE(level->traceComponent(i));
            TRACE(printf("CONSIDERING %d (%d)\n", i, component.getSize()));

            analyzeComponent(component, INITIAL_STATE_MASK);
            
            TRACE(Colorer::print<WHITE>("Found %d solution%c\n", component.getSolutionCount(), component.getSolutionCount() == 1 ? ' ' : 's'));

            TRACE(
                if (component.getSolutionCount() > Debug::mostSolutions)
                {
                    Debug::mostSolutions = component.getSolutionCount();
                }
            )

            if (component.getSolutionCount() == 1)
            {
                TRACE(Colorer::print<YELLOW>("Component %d has ONLY ONE Solution! Solution state mask: %d\n", i, component.getSolutions()->begin()->first));
            }
            else if (component.getSolutionCount() == 0)
            {
                TRACE(Colorer::print<YELLOW>("Component %d is SPECIAL!\n", getComponentCurrentIndex(), i));

                TRACE(level->traceComponent(i));

                level->addSpecialComponent(&component, i);
            }
        }
    }
}

void Analyzer::analyzeComponent(Component& component, int stateMask)
{
    if (component.getSize() == 0)
    {
        if (solutionPathHolder.size() > 0)
        {
            TRACE(
                Colorer::print<WHITE>("Oh yeah! Found full solution!  ");
                level->traceComponent(componentCurrentIndex);
                for (size_t i = 0; i < solutionPathHolder.size(); i++)
                {
                    const Exit* a = solutionPathHolder[i].getStart();
                    const Exit* b = solutionPathHolder[i].getFinish();
                    Colorer::print<WHITE>("(%d,%d,%d) --> (%d,%d,%d)  "
                        , a->getX(), a->getY(), a->getDir()
                        , b->getX(), b->getY(), b->getDir());
                }

                printf("\n");
            )

            SolutionMap solution;
            SolutionMap* solutionFollower = &solution;

            for (size_t i = 0; i < solutionPathHolder.size(); i++)
            {
                (*solutionFollower)[solutionStateMaskHolder[i]][solutionPathHolder[i]] = ComponentSolution();
                solutionFollower = (*solutionFollower)[solutionStateMaskHolder[i]].begin()->second.getSolutions();
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
        for (size_t i = 0; i < component.getExits().size(); i++)
        {
            if (((1 << i) & stateMask) == 0) // if exit is (supposedly) free
            {
                const Exit* e = component.getExitByIndex(i);
                int dir = e->getDir();

                Cell* cell = level->getCell(e->getY(), e->getX());

                int lastDepthCopy = prevDepth;
                prevDepth = depth;
                previousExit.push_back(e);
                previousStateMask.push_back(stateMask);

                backtrack(cell, dir ^ 2);

                previousExit.pop_back();
                previousStateMask.pop_back();
                prevDepth = lastDepthCopy;
            }
        }

        /*FOREACH(component.getExits(), it)
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
        }*/
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
    // Split free cells into components with Flood-fill
    // Additionally initialize all Exit's
    findComponents();

    // Assign opposing exit to every existing one
    findOpposingExits();

    TRACE
    (
        level->traceComponent();
    );

    // Analyzing obstacles ------------------------------------------------------------
    // Find all 'next-touches' for every cell
    findTouchingObstacles();

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

    TRACE(cell->setMark(tracer.depth));
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
    TRACE(cell->setMark(0));
}

bool Analyzer::reachedFinalCell(Cell* cell, int dir) const
{
    int t = cell->getComponentId();

    if (cell->hasExits())
    {
        if (cell->getNextCell(dir)->isFree())
        {
            if (cell->hasExit(dir))
            {
                return true;
            }
        }
        else
        {
            // Look left / right
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
    }
    return false;
}

bool Analyzer::potentialSolution(Cell* cell, int dir) const
{
    if (level->getTemporaryEnds().size() > 0)
    {
        return false;
    }

    return true;
}

void Analyzer::collectResults(const Exit* exit1, const Exit* exit2)
{
    //level->traceComponent(getComponentCurrentIndex());

    Path path(exit1, exit2, depth - prevDepth);
    solutionPathHolder.push_back(path);

    //int exitIndex1 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(exit1);
    //int exitIndex2 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(exit2);
    //assert((previousStateMask.back() & (1 << exitIndex1)) == 0 && (previousStateMask.back() & (1 << exitIndex2)) == 0 && "Erroneous state mask!");
    //int newStateMask = previousStateMask.back() | (1 << exitIndex1) | (1 << exitIndex2);
    //solutionStateMaskHolder.push_back(newStateMask);
    solutionStateMaskHolder.push_back(previousStateMask.back());
}

void Analyzer::uncollectResults()
{
    solutionPathHolder.pop_back();
    solutionStateMaskHolder.pop_back();
}

void Analyzer::solutionFound(Cell* cell, int dir)
{
    //level->traceComponent();

    // Assuming cell->hasExits()
    
    if (cell->getNextCell(dir)->isObstacle())
    {
        // Try left / right
        int leftDirection = Left[dir];
        if (cell->getNextCell(leftDirection)->isFree())
        {
            if (cell->hasExit(leftDirection))
            {
                collectResults(previousExit.back(), cell->getExit(leftDirection));

                int exitIndex1 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(previousExit.back());
                int exitIndex2 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(leftDirection));
                assert((previousStateMask.back() & (1 << exitIndex1)) == 0 && (previousStateMask.back() & (1 << exitIndex2)) == 0 && "Erroneous state mask!");
                int newStateMask = previousStateMask.back() | (1 << exitIndex1) | (1 << exitIndex2);

                preOccupyAction(cell, dir);
                occupy(cell, dir);
                postOccupyAction(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                preRestoreAction(cell, dir);
                restore(cell, dir);
                postRestoreAction(cell, dir);

                uncollectResults();
            }
            else
            {
                // continue: turn left
                TRACE(tracer.depth--); // TODO: rethink ugly hack, because backtrack() is called a few extra times
                backtrack(cell, leftDirection);
                TRACE(tracer.depth++);
            }
        }
            
        int rightDirection = Right[dir];
        if (cell->getNextCell(rightDirection)->isFree())
        {
            if (cell->hasExit(rightDirection))
            {
                collectResults(previousExit.back(), cell->getExit(rightDirection));

                int exitIndex1 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(previousExit.back());
                int exitIndex2 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(rightDirection));
                assert((previousStateMask.back() & (1 << exitIndex1)) == 0 && (previousStateMask.back() & (1 << exitIndex2)) == 0 && "Erroneous state mask!");
                int newStateMask = previousStateMask.back() | (1 << exitIndex1) | (1 << exitIndex2);

                preOccupyAction(cell, dir);
                occupy(cell, dir);
                postOccupyAction(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                preRestoreAction(cell, dir);
                restore(cell, dir);
                postRestoreAction(cell, dir);

                uncollectResults();
            }
            else
            {
                // continue: turn right
                TRACE(tracer.depth--);
                backtrack(cell, rightDirection);
                TRACE(tracer.depth++);
            }
        }
    }
    else
    {
        if (cell->hasExit(dir))
        {
            // TODO: this condition seems to be true overall
            if (cell->getNextCell(dir)->isFree())
            {
                collectResults(previousExit.back(), cell->getExit(dir));

                int exitIndex1 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(previousExit.back());
                int exitIndex2 = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(dir));
                assert((previousStateMask.back() & (1 << exitIndex1)) == 0 && (previousStateMask.back() & (1 << exitIndex2)) == 0 && "Erroneous state mask!");
                int newStateMask = previousStateMask.back() | (1 << exitIndex1) | (1 << exitIndex2);

                preOccupyAction(cell, dir);
                occupy(cell, dir);
                postOccupyAction(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                preRestoreAction(cell, dir);
                restore(cell, dir);
                postRestoreAction(cell, dir);

                uncollectResults();
            }

            // Consider case: cell ahead has been occupied somewhen during Analysis
            {
                // Update latest stateMask: mark exit ahead as necessary occupied
                int exitIndex = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(dir));
                int prevMask = previousStateMask.back();
                previousStateMask.pop_back();
                assert(((1 << exitIndex) & prevMask) == 0 && "Erroneous state mask!");
                int newMask = prevMask | (1 << exitIndex);
                previousStateMask.push_back(newMask);

                int leftDirection = Left[dir];
                int rightDirection = Right[dir];

                if (cell->getNextCell(leftDirection)->isFree())
                {
                    TRACE(tracer.depth--);
                    backtrack(cell, leftDirection);
                    TRACE(tracer.depth++);
                }
                if (cell->getNextCell(rightDirection)->isFree())
                {
                    TRACE(tracer.depth--);
                    backtrack(cell, rightDirection);
                    TRACE(tracer.depth++);
                }

                previousStateMask.pop_back();
                previousStateMask.push_back(prevMask);
            }
        }
    }
}
