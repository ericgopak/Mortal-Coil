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
            const std::vector<const Exit*>& exits = component.getExits();

            if (component.getExits().size() == 2)
            {
                const Exit* e1 = *exits.begin();
                const Exit* e2 = *(++exits.begin());

                SolutionHead h1 = {e1->getX(), e1->getY(), e1->getDir()};
                SolutionHead h2 = {e2->getX(), e2->getY(), e2->getDir()};
                
                SolutionBody b1 = {e2->getX(), e2->getY(), e2->getDir()};
                SolutionBody b2 = {e1->getX(), e1->getY(), e1->getDir()};

                SolutionRecord solution1(0, h1, b1);
                SolutionRecord solution2(0, h2, b2);

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getSolutions()->addSolution(v1);
                component.getSolutions()->addSolution(v2);
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
            TRACE(printf("CONSIDERING %d (size: %d  exits: %d  exitCells: %d)\n", i, component.getSize(), component.getExits().size(), component.getExitCells().size()));

            analyzeComponent(component, INITIAL_STATE_MASK);

            TRACE(Colorer::print<WHITE>("Component %d got %d solution%c\n", i, component.getSolutionCount(), component.getSolutionCount() == 1 ? ' ' : 's'));

#ifdef TRACE_STATISTICS
            if (component.getSolutionCount() > Debug::mostSolutions)
            {
                Debug::mostSolutions = component.getSolutionCount();
            }
#endif
            if (component.getSolutionCount() == 1)
            {
                TRACE(Colorer::print<YELLOW>("Component %d has ONLY ONE Solution!\n", i));
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
    if (component.getOccupiedCount() == component.getSize())
    {
        if (solutionRecordHolder.size() > 0)
        {
            component.getSolutions()->addSolution(solutionRecordHolder);

            TRACE(
                level->traceComponent(componentCurrentIndex);
                Colorer::print<WHITE>("Oh yeah! Found full solution %d:    ", component.getSolutionCount());
                for (size_t i = 0; i < solutionRecordHolder.size(); i++)
                {
                    const SolutionRecord& record = solutionRecordHolder[i];
                    Colorer::print<WHITE>("[%d] --> (%d,%d,%d) --> (%d,%d,%d)   "
                        , std::get<0>(record)
                        , std::get<1>(record).startX, std::get<1>(record).startY, std::get<1>(record).startDir
                        , std::get<2>(record).endX, std::get<2>(record).endY, std::get<2>(record).endDir
                    );
                }
                printf("\n");
            );
        }
        else
        {
            assert(false && "Not supposed to happen!");
        }
    }
    else
    {
        TRACE(tracer.layer++);

        // Consider through solutions
        for (size_t i = 0; i < component.getExits().size(); i++)
        {
            if (((1 << i) & stateMask) == 0) // if exit is (supposedly) free
            {
                const Exit* e = component.getExitByIndex(i);
                int dir = e->getDir();

                Cell* cell = level->getCell(e->getY(), e->getX());
                Cell* prevCell = cell->getNextCell(dir);
                if (prevCell->isObstacle() == false && prevCell->isFree()) // if exit is actually free
                {
                    int lastDepthCopy = prevDepth;
                    prevDepth = depth;
                    previousExit.push_back(e);
                    previousStateMask.push_back(stateMask);

                    prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                    prevCell->setFree(false);
                    backtrack(cell, dir ^ 2);
                    prevCell->setFree(true);
                    prevCell->setType(false);

                    previousExit.pop_back();
                    previousStateMask.pop_back();
                    prevDepth = lastDepthCopy;
                }
            }
        }

        TRACE(tracer.layer--);
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

    level->setMostCells(best);
    level->setBiggest(bestid);

    findComponentExits();

    // Assign opposing exit to every existing one
    findOpposingExits();

    TRACE(level->traceComponent());

    // Find all 'next-touches' for every cell
    findTouchingObstacles();

#ifdef TRACE_STATISTICS
    const Component& comp = level->getComponents()[bestid];
    Colorer::print<WHITE>("Biggest component: %d cells  %d exits  %d exitCells ID: %d\n", best, comp.getExits().size(), comp.getExitCells().size(), bestid);
    TRACE(level->traceComponent(bestid));
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

    TRACE(cell->setDepth(tracer.depth));
    TRACE(cell->setLayer(tracer.layer));
}

void Analyzer::postOccupyAction(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* behind = cell->getNextCell(dir ^ 2);
    if (left->isTemporaryEnd())
    {
        level->addTemporaryEnd(left);
        if (left->getComponentId() == cell->getComponentId())
        {
            level->addTemporaryEndsInCurrentComponent(left);
        }
    }
    if (right->isTemporaryEnd())
    {
        level->addTemporaryEnd(right);
        if (right->getComponentId() == cell->getComponentId())
        {
            level->addTemporaryEndsInCurrentComponent(right);
        }
    }
    if (behind->isTemporaryEnd()) // May happen to the first cell in a line
    {
        level->addTemporaryEnd(behind);
        if (behind->getComponentId() == cell->getComponentId())
        {
            level->addTemporaryEndsInCurrentComponent(behind);
        }
    }
}

void Analyzer::preRestoreAction(Cell* cell, int dir) const
{
    const Cell* left  = cell->getNextCell(Left[dir]);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* behind = cell->getNextCell(dir ^ 2);
    if (left->isTemporaryEnd())
    {
        level->removeTemporaryEnd(left);
        if (left->getComponentId() == cell->getComponentId())
        {
            level->removeTemporaryEndsInCurrentComponent(left);
        }
    }
    if (right->isTemporaryEnd())
    {
        level->removeTemporaryEnd(right);
        if (right->getComponentId() == cell->getComponentId())
        {
            level->removeTemporaryEndsInCurrentComponent(right);
        }
    }
    if (behind->isTemporaryEnd()) // May happen to the first cell in a line
    {
        level->removeTemporaryEnd(behind);
        if (behind->getComponentId() == cell->getComponentId())
        {
            level->removeTemporaryEndsInCurrentComponent(behind);
        }
    }
}

void Analyzer::postRestoreAction(Cell* cell, int dir) const
{
    TRACE(cell->setDepth(-1));
    TRACE(cell->setLayer(-1));
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
    if (level->getTemporaryEndsInCurrentComponent().size() > 1) // One end for solutions terminating in this component
    {
        return false;
    }

    if (level->getTemporaryEnds().size() > 2)
    {
        return false;
    }

    return true;
}

void Analyzer::collectResults(const Exit* exit1, const Exit* exit2)
{
    SolutionHead head = {exit1->getX(), exit1->getY(), exit1->getDir()};
    SolutionBody body = {exit2->getX(), exit2->getY(), exit2->getDir()};
    SolutionRecord record(previousStateMask.back(), head, body);
    solutionRecordHolder.push_back(record);
}

void Analyzer::uncollectResults()
{
    solutionRecordHolder.pop_back();
}

void Analyzer::solutionFound(Cell* cell, int dir)
{
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

                cell = moveForward(cell, leftDirection);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                cell = moveBackwards(cell, leftDirection);

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


                cell = moveForward(cell, rightDirection);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                cell = moveBackwards(cell, rightDirection);

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

                const Component& comp = level->getComponents()[getComponentCurrentIndex()];
                int exitIndex1 = comp.getIndexByExit(previousExit.back());
                int exitIndex2 = comp.getIndexByExit(cell->getExit(dir));
                assert((previousStateMask.back() & (1 << exitIndex1)) == 0 && (previousStateMask.back() & (1 << exitIndex2)) == 0 && "Erroneous state mask!");
                int newStateMask = previousStateMask.back() | (1 << exitIndex1) | (1 << exitIndex2);

                cell = moveForward(cell, dir);
                analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
                cell = moveBackwards(cell, dir);

                uncollectResults();
            }

            // Consider case: cell ahead has been occupied somewhen during Analysis
            if (cell->getNextCell(dir)->isFree())
            {
                cell->getNextCell(dir)->setFree(false); // Otherwise loops infinitely (turning left,right,left,...)

                // Update latest stateMask: mark exit ahead as necessary occupied
                int exitIndex = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(dir));
                const int prevMask = previousStateMask.back();
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

                cell->getNextCell(dir)->setFree(true);
            }
        }
    }
}
