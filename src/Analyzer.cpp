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
                const Exit* e1 = exits[0];
                const Exit* e2 = exits[1];

                std::string decision1;
                std::string decision2;
                if ((e1->getDir() ^ 2) != e2->getDir())
                {
                    decision1 += Direction[e2->getDir()];
                }
                if ((e2->getDir() ^ 2) != e1->getDir())
                {
                    decision2 += Direction[e1->getDir()];
                }

                SolutionHead h1 = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                SolutionHead h2 = {e2->getX(), e2->getY(), e2->getDir() ^ 2};
                
                SolutionBody b1 = {e2->getX(), e2->getY(), e2->getDir(), decision1};
                SolutionBody b2 = {e1->getX(), e1->getY(), e1->getDir(), decision2};

                SolutionRecord solution1(1 << 0, h1, b1);
                SolutionRecord solution2(1 << 1, h2, b2);

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getSolutions()->addSolution(v1, 0, 0);
                component.getSolutions()->addSolution(v2, 0, 0);
            }
            else if (component.getExits().size() == 1)
            {
                const Exit* e = *exits.begin();

                TRACE(Colorer::print<YELLOW>("Double check: end at (%d, %d)  (one exit only)\n", e->getX(), e->getY()));

                std::string decision1;
                decision1 += Direction[e->getDir()];
                std::string decision2; // Do nothing

                SolutionHead h1 = {e->getX(), e->getY(), e->getDir()};
                SolutionHead h2 = {e->getX(), e->getY(), e->getDir() ^ 2};
                
                SolutionBody b1 = {e->getX(), e->getY(), e->getDir()    , decision1};
                SolutionBody b2 = {e->getX(), e->getY(), e->getDir() ^ 2, decision2};

                SolutionRecord solution1(0, h1, b1);
                SolutionRecord solution2(1, h2, b2); // Exit must have been occupied

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getSolutions()->addSolution(v1, 1, 0);
                component.getSolutions()->addSolution(v2, 0, 1);
            }
            else
            {
                assert(false && "Should not ever happen!");
            }
        }
        else if (component.getSize() > 1)
        {
            setComponentCurrentIndex(i);

            TRACE(level->traceComponent(i));
            TRACE(printf("CONSIDERING %d (size: %d  exits: %d  exitCells: %d)\n", i, component.getSize(), component.getExits().size(), component.getExitCells().size()));

            assert(solutionRecordHolder.size() == 0);
            analyzeComponent(component, INITIAL_STATE_MASK);

            TRACE(Colorer::print<WHITE>("Component %d got %d solution%c\n", i, component.getSolutionCount(), component.getSolutionCount() == 1 ? ' ' : 's'));

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

#ifdef TRACE_STATISTICS
        Debug::totalSolutions += component.getSolutionCount();
        if (component.getSolutionCount() > Debug::mostSolutions)
        {
            Debug::mostSolutions = component.getSolutionCount();
        }
#endif
    }
}

void Analyzer::analyzeComponent(Component& component, int stateMask)
{
    if (component.getOccupiedCount() == component.getSize())
    {
        if (solutionRecordHolder.size() > 0)
        {
            int solutionNumber = component.getSolutionCount() + 1;
            TRACE(
                level->traceComponent(componentCurrentIndex);
                Colorer::print<WHITE>("Oh yeah! Found full solution %d:    ", solutionNumber);
                for (size_t i = 0; i < solutionRecordHolder.size(); i++)
                {
                    const SolutionRecord& record = solutionRecordHolder[i];
                    Colorer::print<WHITE>("[%d] --> (%d,%d,%d) --> (%d,%d,%d)  [%s]    "
                        , std::get<0>(record)
                        , std::get<1>(record).startX, std::get<1>(record).startY, std::get<1>(record).startDir
                        , std::get<2>(record).endX, std::get<2>(record).endY, std::get<2>(record).endDir
                        , std::get<2>(record).solution.c_str()
                    );
                }
                printf("\n");
            );

            bool isStarting = true;
            bool isEnding = false;

            const int& startX   = std::get<1>(solutionRecordHolder[0]).startX;
            const int& startY   = std::get<1>(solutionRecordHolder[0]).startY;
            const int& startDir = std::get<1>(solutionRecordHolder[0]).startDir;
            if (level->getCell(startY, startX)->hasExit(startDir) && level->getCell(startY, startX)->isTemporaryEnd()) // TODO: test
            {
                isStarting = false;
            }

            const int& endX   = std::get<2>(solutionRecordHolder[solutionRecordHolder.size() - 1]).endX;
            const int& endY   = std::get<2>(solutionRecordHolder[solutionRecordHolder.size() - 1]).endY;
            const int& endDir = std::get<2>(solutionRecordHolder[solutionRecordHolder.size() - 1]).endDir;
            if (level->getCell(endY, endX)->hasExit(endDir) == false) // TODO: test
            {
                assert(component.getOccupiedCount() == component.getSize());
                isEnding = true;
            }

            component.getSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
        }
        else
        {
            assert(false && "Not supposed to happen!");
        }
    }
    else
    {
        TRACE(tracer.layer++);

        decisionHolder.push_back("");

        // Consider starting solutions
        if (solutionRecordHolder.size() == 0)
        {
            FOREACH(component.getCells(), c)
            {
                Cell* cell = const_cast<Cell*>(*c); // UGLY HACK!!! TODO: Reconsider!!!

int x = cell->getX();
int y = cell->getY();

                for (int dir = 0; dir < 4; dir++)
                {
                    if (cell->getNextCell(dir)->isObstacle() == false
                        //&& cell->getComponentId() == cell->getNextCell(dir)->getComponentId() TODO: what was this condition for???
                        )
                    {
                        assert(cell->getNextCell(dir)->isFree());
                        assert(stateMask == 0);

                        int lastDepthCopy = prevDepth; // TODO: consider storing in outer stack
                        prevDepth = depth;

                        SolutionHead head = {cell->getX(), cell->getY(), dir};

                        previousHead.push_back(head);

                        Cell* prevCell = cell->getNextCell(dir ^ 2);

                        // Non-starting solution
                        if (cell->hasExit(dir ^ 2))
                        {
                            int exitIndex = level->getComponents()[cell->getComponentId()].getIndexByExit(cell->getExit(dir ^ 2));
                            previousStateMask.push_back(stateMask | (1 << exitIndex));
                            prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                            prevCell->setFree(false);
                            backtrack(cell, dir);
                            prevCell->setType(false);
                            prevCell->setFree(true);
                            previousStateMask.pop_back();
                        }

                        // Starting solution
                        if (prevCell->isObstacle() || level->getComponents()[prevCell->getComponentId()].getSize() != 1) // do not allow such positions to be solution heads
                        {
                            decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);

                            previousStateMask.push_back(stateMask);
                            backtrack(cell, dir);
                            // TODO: consider pushing this cell onto temporaryEnds!
                            previousStateMask.pop_back();

                            decisionHolder[decisionHolder.size() - 1].pop_back();
                        }

                        //if (cell->hasExit(dir ^ 2) && level->getComponents()[prevCell->getComponentId()].getSize() == 1)
                        //{
                        //    int exitIndex = level->getComponents()[cell->getComponentId()].getIndexByExit(cell->getExit(dir ^ 2));
                        //    previousStateMask.push_back(stateMask | (1 << exitIndex)); // Non-starting solution
                        //    prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                        //    prevCell->setFree(false);
                        //    backtrack(cell, dir);
                        //    prevCell->setType(false);
                        //    prevCell->setFree(true);
                        //    previousStateMask.pop_back();
                        //}
                        //else // Starting solution
                        //{
                        //    decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);

                        //    previousStateMask.push_back(stateMask);
                        //    backtrack(cell, dir);
                        //    // TODO: consider pushing this cell onto temporaryEnds!
                        //    previousStateMask.pop_back();

                        //    decisionHolder[decisionHolder.size() - 1].pop_back();
                        //}

                        previousHead.pop_back();
                        prevDepth = lastDepthCopy;
                    }
                }
            }
        }
        else // Consider non-starting solutions.
        {
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

                        SolutionHead head = {cell->getX(), cell->getY(), dir ^ 2}; // TODO: does it leave a copy on the stack? What about @previousHead.push_back({...});@ ?

                        previousHead.push_back(head);
                        previousStateMask.push_back(stateMask | (1 << i)); // Through solution: mask the exit out

                        prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                        prevCell->setFree(false);
                        backtrack(cell, dir ^ 2);
                        prevCell->setFree(true);
                        prevCell->setType(false);

                        previousHead.pop_back();
                        previousStateMask.pop_back();
                        prevDepth = lastDepthCopy;
                    }
                }
            }
        }

        decisionHolder.pop_back();

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

void Analyzer::backtrack(Cell* cell, int direction)
{
    if (!shouldConsider(cell, direction))
    {
        return;
    }

    TRACE(tracer.depth++);

    //level->traceComponent();

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
                    decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

                    cell = moveForward(cell, leftDirection);
                    backtrack(cell, leftDirection);
                    cell = moveBackwards(cell, leftDirection);

                    decisionHolder[decisionHolder.size() - 1].pop_back();
                }

                if (stopBacktracking() == false) // Do not terminate in order to guarantee proper postAction()
                {
                    // Turn right
                    int rightDirection = Right[direction];
                    if (cell->getNextCell(rightDirection)->isFree())
                    {
                        decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

                        cell = moveForward(cell, rightDirection);
                        backtrack(cell, rightDirection);
                        cell = moveBackwards(cell, rightDirection);

                        decisionHolder[decisionHolder.size() - 1].pop_back();
                    }
                }
            }
        }
    }

    postAction(cell, direction);

    TRACE(tracer.depth--);
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
    // Very last free cell in the component
    const Component* comp = &level->getComponents()[cell->getComponentId()];
    if (comp->getOccupiedCount() + 1 == comp->getSize())
    {
        return true;
    }

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

void Analyzer::collectResults(const SolutionHead& head, const SolutionBody& body)
{
    SolutionRecord record(previousStateMask.back(), head, body);
    solutionRecordHolder.push_back(record);
}

void Analyzer::uncollectResults()
{
    solutionRecordHolder.pop_back();
}

void Analyzer::proceedAnalyzing(Cell* cell, int dir)
{
    const Exit* outExit = cell->getExit(dir);
    assert(outExit != NULL);

    SolutionBody body = {outExit->getX(), outExit->getY(), outExit->getDir(), decisionHolder.back()};
    const SolutionHead& head = previousHead.back();

    collectResults(head, body);

    int outExitIndex = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(outExit);

    assert((previousStateMask.back() & (1 << outExitIndex)) == 0 && "Erroneous state mask!");
    int newStateMask = previousStateMask.back() | (1 << outExitIndex);

    cell = moveForward(cell, dir);
    analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
    cell = moveBackwards(cell, dir);

    uncollectResults();
}

void Analyzer::solutionFound(Cell* cell, int dir)
{
    // Very last free cell
    Component& comp = level->getComponents()[cell->getComponentId()];
    if (comp.getOccupiedCount() + 1 == comp.getSize()) // Ending solution
    {
        // Occupy and finish
        occupy(cell, dir);

        SolutionBody body = {cell->getX(), cell->getY(), dir, decisionHolder.back()};
        const SolutionHead& head = previousHead.back();

        int prevMask = previousStateMask.back();

        int  leftDirection = Left[dir];
        int rightDirection = Right[dir];
        if (cell->hasExit(leftDirection) && cell->getNextCell(leftDirection)->isFree()) // exit on the left
        {
            int newStateMask = prevMask;
            previousStateMask.pop_back();

            int exitIndex = comp.getIndexByExit(cell->getExit(leftDirection));
            assert((prevMask & (1 << exitIndex)) == 0);
            newStateMask |= 1 << exitIndex;
            previousStateMask.push_back(newStateMask);
        }
        if (cell->hasExit(rightDirection) && cell->getNextCell(rightDirection)->isFree()) // exit on the right
        {
            int newStateMask = prevMask;
            previousStateMask.pop_back();

            int exitIndex = comp.getIndexByExit(cell->getExit(rightDirection));
            assert((prevMask & (1 << exitIndex)) == 0);
            newStateMask |= 1 << exitIndex;
            previousStateMask.push_back(newStateMask);
        }

        collectResults(head, body);
        analyzeComponent(comp, previousStateMask.back()); // TODO: replace mask with -1
        uncollectResults();

        {
            previousStateMask.pop_back();
            previousStateMask.push_back(prevMask);
        }

        restore(cell, dir);
    }

    // Otherwise assuming cell->hasExits()
    if (cell->getNextCell(dir)->isObstacle())
    {
        // Try left / right
        int leftDirection = Left[dir];
        if (cell->getNextCell(leftDirection)->isFree())
        {
            decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

            if (cell->hasExit(leftDirection))
            {
                proceedAnalyzing(cell, leftDirection);
            }
            else
            {
                // continue: turn left
                TRACE(tracer.depth--); // TODO: rethink ugly hack, because backtrack() is called a few extra times
                                       // What about moving preAction() trace code to moveForward()?
                backtrack(cell, leftDirection);
                TRACE(tracer.depth++);
            }

            decisionHolder[decisionHolder.size() - 1].pop_back();
        }
            
        int rightDirection = Right[dir];
        if (cell->getNextCell(rightDirection)->isFree())
        {
            decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

            if (cell->hasExit(rightDirection))
            {
                proceedAnalyzing(cell, rightDirection);
            }
            else
            {
                // continue: turn right
                TRACE(tracer.depth--);
                backtrack(cell, rightDirection);
                TRACE(tracer.depth++);
            }

            decisionHolder[decisionHolder.size() - 1].pop_back();
        }
    }
    else
    {
        if (cell->hasExit(dir))
        {
            // TODO: this condition seems to be true overall
            if (cell->getNextCell(dir)->isFree())
            {
                proceedAnalyzing(cell, dir);
            }

            // Consider case: cell ahead has been occupied somewhen during Analysis
            if (cell->getNextCell(dir)->isFree())
            {
                cell->getNextCell(dir)->setFree(false); // Otherwise loops infinitely (turning left,right,left,...)

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
                    decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

                    TRACE(tracer.depth--);
                    backtrack(cell, leftDirection);
                    TRACE(tracer.depth++);

                    decisionHolder[decisionHolder.size() - 1].pop_back();
                }
                if (cell->getNextCell(rightDirection)->isFree())
                {
                    decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

                    TRACE(tracer.depth--);
                    backtrack(cell, rightDirection);
                    TRACE(tracer.depth++);

                    decisionHolder[decisionHolder.size() - 1].pop_back();
                }

                previousStateMask.pop_back();
                previousStateMask.push_back(prevMask);

                cell->getNextCell(dir)->setFree(true);
            }
        }
    }
}
