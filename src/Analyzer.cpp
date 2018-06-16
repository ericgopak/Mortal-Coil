#include "Common.h"
#include "Colorer.h"
#include "Component.h"
#include "Analyzer.h"

// Using FNV_hash and XORing pointer addresses
//size_t SolutionFragmentHasher::operator() (const std::vector<UniqueSolutionFragment>& fragmentVector) const
//{
//    size_t hash = 2166136261; // FNV_offset_basis
//    for (size_t i = 0; i < fragmentVector.size(); i++)
//    {
//        hash = hash ^ (size_t)fragmentVector[i].first ^ (size_t)fragmentVector[i].second;
//        hash = hash * 16777619;
//    }
//    return hash;
//}

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

// Get all solutions for 1-sized components
static void generateAllSimpleSolutions(Component* component)
{
    for (size_t i = 0; i < component->getExits().size(); i++)
    {
        for (size_t j = 0; j < component->getExits().size(); j++)
        {
            if (i == j)
            {
                continue;
            }

            // Non-starting solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];
                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
				std::string decision;
				//std::string qdecision;
                if ((e1->getDir() ^ 2) != e2->getDir())
                {
                    decision = Direction[e2->getDir()];
					/*if (e2->getHostCell()->hasExit(e2->getDir() ^ 2))
						qdecision = Direction[e2->getDir()];*/
                }

				/*std::vector<const Exit*> exitSequence;
				exitSequence.push_back(e1);
				exitSequence.push_back(e2);*/

                // TODO: consider mask ~((1 << i) | (1 << j))
                MustBeBlockedMask mustBeBlockedMask = 1 << i; // TODO: could also be 0 -> unify with Solver & Analyzer
                MustBeFreeMask mustBeFreeMask = 1 << j;

                //SolutionBody body = {e2->getX(), e2->getY(), e2->getDir(), decision};
                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
				SolutionBody body = { e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision };

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getNonStartingSolutions()->addSolution(v, 0, 0);
            }

            // Starting solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];

                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                std::string decision;
				decision += Direction[e2->getDir()];
				/*std::string qdecision;
				qdecision += Direction[e2->getDir()];*/

				/*std::vector<const Exit*> exitSequence;
				exitSequence.push_back(e2);*/

                MustBeBlockedMask mustBeBlockedMask = 0;
                MustBeFreeMask mustBeFreeMask = (1 << j) | (1 << i); // This distinguishes starting from through solutions

                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
				SolutionBody body = { e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision };

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getStartingSolutions()->addSolution(v, 1, 0); // Is starting, not ending
            }

            // Ending solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];

                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                std::string decision; // Do nothing
				//std::string qdecision; // Do nothing

				std::vector<const Exit*> exitSequence;
				/*exitSequence.push_back(e1);*/

                MustBeBlockedMask mustBeBlockedMask = (1 << component->getExits().size()) - 1; // ~0
                MustBeFreeMask mustBeFreeMask = 0;

                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
				SolutionBody body = { e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision };

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getNonStartingSolutions()->addSolution(v, 0, 1); // Ending, not starting
            }
        }
    }
}

void Analyzer::analyzeComponents()
{
    for (int i = 0; i < level->getComponentCount(); i++)
    {
        uniqueSolutions.clear();

        Component& component = level->getComponents()[i];

        if (component.getSize() == 1)
        {
            const std::vector<const Exit*>& exits = component.getExits();

            if (exits.size() == 1)
            {
                const Exit* e = *exits.begin();

                TRACE(Colorer::print<YELLOW>("Double check: end at (%d, %d)  (one exit only)\n", e->getX(), e->getY()));

                std::string decision1;
                decision1 += Direction[e->getDir()];
                std::string decision2; // Do nothing
				/*std::vector<const Exit*> exitSequence1;
				std::vector<const Exit*> exitSequence2;
				exitSequence1.push_back(e);*/

                SolutionHead h1 = {e->getX(), e->getY(), e->getDir()};
                SolutionHead h2 = {e->getX(), e->getY(), e->getDir() ^ 2};

                MustBeBlockedMask mustBeBlockedMask1 = 0;
                MustBeFreeMask mustBeFreeMask1 = 1 << 0;
                MustBeBlockedMask mustBeBlockedMask2 = 1 << 0;
                MustBeFreeMask mustBeFreeMask2 = 0;

                StateMask stateChange1 = 1 << 0; // ~0
                StateMask stateChange2 = 1 << 0; // ~0

				SolutionBody b1 = { e->getX(), e->getY(), e->getDir(), mustBeBlockedMask1, mustBeFreeMask1, stateChange1, decision1 };
				SolutionBody b2 = { e->getX(), e->getY(), e->getDir() ^ 2, mustBeBlockedMask2, mustBeFreeMask2, stateChange2, decision2 };

                // Starting solution
                SolutionRecord solution1(mustBeBlockedMask1, mustBeFreeMask1, h1, b1); // Exit must be free
                // Ending solution
                SolutionRecord solution2(mustBeBlockedMask2, mustBeFreeMask2, h2, b2); // Exit must have been occupied

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getStartingSolutions()->addSolution(v1, 1, 0);
                component.getNonStartingSolutions()->addSolution(v2, 0, 1);
            }
            else if (exits.size() == 2)
            {
                generateAllSimpleSolutions(&component);
            }
            else if (exits.size() == 3) // Single-cell component with 3 exits
            {
                generateAllSimpleSolutions(&component);
            }
            else if (exits.size() == 4)
            {
                throw std::exception("Found single-cell component with 4 exits! Congrats!");
            }
        }
        else if (component.getSize() > 1)
        {
            setComponentCurrentIndex(i);

            TRACE(level->traceComponent(i));
            TRACE(printf("CONSIDERING %d (size: %d  exits: %d  exitCells: %d)\n", i, component.getSize(), component.getExits().size(), component.getExitCells().size()));

            assert(solutionRecordHolder.size() == 0);
            analyzeComponent(component);

            TRACE(Colorer::print<WHITE>("Component %d got %d through, %d starting, %d non-starting and %d ending solutions\n", i, component.getThroughSolutionCount(), component.getStartingSolutionCount(), component.getNonStartingSolutionCount(), component.getEndingSolutionCount()));

            if (component.getStartingSolutionCount() == 0)
            {
                TRACE(Colorer::print<YELLOW>("Component %d CANNOT be STARTING!\n", i));
            }
            else if (component.getEndingSolutionCount() == 0)
            {
                TRACE(Colorer::print<YELLOW>("Component %d CANNOT be ENDING!\n", i));
            }

            if (component.getThroughSolutionCount() == 1)
            {
                TRACE(Colorer::print<YELLOW>("Component %d has ONLY ONE through Solution!\n", i));
            }
            else if (component.getThroughSolutionCount() == 0)
            {
                TRACE(Colorer::print<YELLOW>("Component %d is SPECIAL!\n", getComponentCurrentIndex(), i));
                TRACE(level->traceComponent(i));

                level->addSpecialComponent(&component, i);
            }
        }

#ifdef TRACE_STATISTICS
        Debug::totalSolutionsCounter += component.getTotalSolutionCount();
        Debug::startingSolutionsCounter += component.getStartingSolutionCount();
        Debug::nonStartingSolutionsCounter += component.getNonStartingSolutionCount();
        Debug::throughSolutionsCounter += component.getThroughSolutionCount();
        Debug::endingSolutionsCounter += component.getEndingSolutionCount();
        if (component.getTotalSolutionCount() > Debug::mostSolutions)
        {
            Debug::mostSolutions = component.getTotalSolutionCount();
        }
#endif
    }
}

bool Analyzer::solutionIsStarting(const std::vector<SolutionRecord>& solution) const
{
    bool isStarting = true;

    //MustBeBlockedMask mustBeBlockedMask = std::get<0>(solution[0]);
    MustBeBlockedMask mustBeFreeMask = std::get<1>(solution[0]);

    const int& startX   = std::get<2>(solution[0]).startX;
    const int& startY   = std::get<2>(solution[0]).startY;
    const int& startDir = std::get<2>(solution[0]).startDir;

    const Cell* cell = level->getCell(startY, startX);
    const Component* comp = &level->getComponents()[cell->getComponentId()];

    // Exit behind and this exit must not be free
    if (cell->hasExit(startDir ^ 2)
        && (mustBeFreeMask & (1 << comp->getIndexByExit(level->getCell(startY, startX)->getExit(startDir ^ 2)))) == 0)
    {
        isStarting = false;
    }

    return isStarting;
}

bool Analyzer::solutionIsEnding(const std::vector<SolutionRecord>& solution) const
{
    bool isEnding = true;

    const SolutionRecord& lastRecord = solutionRecordHolder[solutionRecordHolder.size() - 1];
    const int& endX   = std::get<3>(lastRecord).endX;
    const int& endY   = std::get<3>(lastRecord).endY;
    const int& endDir = std::get<3>(lastRecord).endDir;

    MustBeBlockedMask mustBeFreeMask = std::get<1>(solution[solution.size() - 1]);

    const Cell* cell = level->getCell(endY, endX);
    const Component* comp = &level->getComponents()[cell->getComponentId()];

    // Exit in front and it must be free
    if (cell->hasExit(endDir) && (mustBeFreeMask & (1 << comp->getIndexByExit(cell->getExit(endDir)))) != 0)
    {
        isEnding = false;
    }

    return isEnding;
}

std::vector<UniqueSolutionFragment> Analyzer::getUniqueSolutionFragments(const std::vector<SolutionRecord>& solution) const
{
    const size_t n = solution.size();

    std::vector<UniqueSolutionFragment> res(n);

    if (n == 0)
    {
        return res;
    }

    // First fragment
    if (solutionIsStarting(solution))
    {
        std::get<0>(res[0]) = NULL;
    }
    else
    {
        const SolutionHead& head = std::get<2>(solution[0]);
        std::get<0>(res[0]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
    }

    if (n > 1)
    {
        const SolutionBody& body = std::get<3>(solution[0]);
        std::get<1>(res[0]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    // Middle fragments
    for (size_t i = 1; i < solution.size() - 1; i++)
    {
        const SolutionHead& head = std::get<2>(solution[i]);
        const SolutionBody& body = std::get<3>(solution[i]);
        std::get<0>(res[i]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
        std::get<1>(res[i]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    if (n > 1)
    {
        // Penultimate fragment
        const SolutionHead& head = std::get<2>(solution[solution.size() - 1]);
        std::get<0>(res[n - 1]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
    }

    // Ultimate fragment
    if (solutionIsEnding(solution))
    {
        std::get<1>(res[n - 1]) = NULL;
    }
    else
    {
        const SolutionBody& body = std::get<3>(solution[solution.size() - 1]);
        std::get<1>(res[n - 1]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    return res;
}

void Analyzer::analyzeComponent(Component& component)
{
    if (component.getOccupiedCount() == component.getSize())
    {
        if (solutionRecordHolder.size() > 0)
        {
            int solutionNumber = component.getTotalSolutionCount() + 1;
            /*TRACE(
                level->traceComponent(componentCurrentIndex);
                Colorer::print<WHITE>("Oh yeah! Found full solution %d:    ", solutionNumber);
                for (size_t i = 0; i < solutionRecordHolder.size(); i++)
                {
                    const SolutionRecord& record = solutionRecordHolder[i];
                    Colorer::print<WHITE>("[%d | %d] --> (%d,%d,%d) --> (%d,%d,%d)  [%s]    "
                        , std::get<0>(record)
                        , std::get<1>(record)
                        , std::get<2>(record).startX, std::get<2>(record).startY, std::get<2>(record).startDir
                        , std::get<3>(record).endX,   std::get<3>(record).endY,   std::get<3>(record).endDir
                        , std::get<3>(record).solution.c_str()
                    );
                }
                printf("\n");
            );*/

            auto newSolution = getUniqueSolutionFragments(solutionRecordHolder);

            if (uniqueSolutions.find(newSolution) == uniqueSolutions.end())
            {
                uniqueSolutions.insert(newSolution);

                bool isStarting = solutionIsStarting(solutionRecordHolder);
                bool isEnding   = solutionIsEnding(solutionRecordHolder);
                //component.getSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
                if (isStarting)
                {
                    component.getStartingSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
                }
                else
                {
                    component.getNonStartingSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
                }
            }
#ifdef TRACE_STATISTICS
            else
            {
                Debug::similarSolutionsCounter++;
            }
#endif
        }
        else
        {
            throw "Not supposed to happen!";
        }
    }
    else
    {
        TRACE(tracer.layer++);

        decisionHolder.push_back("");
        stateChangeStack.push_back(0);

        if (solutionRecordHolder.size() == 0)
        {
            FOREACH(component.getCells(), c)
            {
                Cell* cell = const_cast<Cell*>(*c); // UGLY HACK!!! TODO: Reconsider!!!

                for (int dir = 0; dir < 4; dir++)
                {
                    if (cell->getNextCell(dir)->isObstacle() == false
                        //&& cell->getComponentId() == cell->getNextCell(dir)->getComponentId() TODO: Get rid of this kind of solutions!
                        )
                    {
                        assert(cell->getNextCell(dir)->isFree());

                        int lastDepthCopy = prevDepth; // TODO: consider storing in outer stack
                        prevDepth = depth;

                        SolutionHead head = {cell->getX(), cell->getY(), dir};

                        previousHead.push_back(head);

                        Cell* prevCell = cell->getNextCell(dir ^ 2);

                        // Consider non-starting solutions
                        if (cell->hasExit(dir ^ 2))
                        {
                            int exitIndex = level->getComponents()[cell->getComponentId()].getIndexByExit(cell->getExit(dir ^ 2));

                            mustBeFreeStateMask.push_back(0);
                            mustBeBlockedStateMask.push_back(1 << exitIndex);

                            prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                            prevCell->setFree(false);
                            backtrack(cell, dir);
                            prevCell->setType(false);
                            prevCell->setFree(true);

                            mustBeFreeStateMask.pop_back();
                            mustBeBlockedStateMask.pop_back();
                        }

                        // Consider starting solutions
                        if (cell->hasExit(dir ^ 2)) // Special case: there's an exit behind
                        {
                            int exitIndex = level->getComponents()[cell->getComponentId()].getIndexByExit(cell->getExit(dir ^ 2));
                            decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);
                            mustBeFreeStateMask.push_back(1 << exitIndex);
                            mustBeBlockedStateMask.push_back(0);

                            backtrack(cell, dir);

                            mustBeFreeStateMask.pop_back();
                            mustBeBlockedStateMask.pop_back();
                            decisionHolder[decisionHolder.size() - 1].pop_back();
                        }
                        else if (prevCell->isObstacle() == false && level->getComponents()[prevCell->getComponentId()].getSize() == 1)
                        {
                            // There is no point in starting from the exitCell with 1-sized component behind
                            // Do nothing
                        }
                        else // Normal cases
                        {
                            decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);

                            mustBeFreeStateMask.push_back(0);
                            mustBeBlockedStateMask.push_back(0);

                            backtrack(cell, dir);

                            mustBeFreeStateMask.pop_back();
                            mustBeBlockedStateMask.pop_back();

                            decisionHolder[decisionHolder.size() - 1].pop_back();
                        }

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
                if (component.getExits()[i]->getHostCell()->isFree())
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
                        mustBeFreeStateMask.push_back(0);
                        mustBeBlockedStateMask.push_back(1 << i);

                        prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                        prevCell->setFree(false);

                        backtrack(cell, dir ^ 2);

                        prevCell->setFree(true);
                        prevCell->setType(false);

                        previousHead.pop_back();
                        mustBeFreeStateMask.pop_back();
                        mustBeBlockedStateMask.pop_back();

                        prevDepth = lastDepthCopy;
                    }
                }
            }
        }

        decisionHolder.pop_back();
        stateChangeStack.pop_back();

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

    // Split free cells into components with Flood-fill
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

    // Finds component exits along its outer perimeter
    findComponentExits();

    // TODO: if component is a 'donut' - flood inner cells as well (this way there won't be any inner exits)
    floodInnerSubcomponents();

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
				int rightDirection = Right[direction];

                if (cell->getNextCell(leftDirection)->isFree())
                {
					//bool automaticDecision = !cell->getNextCell(rightDirection)->isFree();
					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

                    cell = moveForward(cell, leftDirection);
                    backtrack(cell, leftDirection);
                    cell = moveBackwards(cell, leftDirection);

					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].pop_back();
                }

                if (stopBacktracking() == false) // Do not terminate in order to guarantee proper postAction()
                {
                    // Turn right
                    if (cell->getNextCell(rightDirection)->isFree())
                    {
						//bool automaticDecision = !cell->getNextCell(leftDirection)->isFree();
						//if (!automaticDecision)
							decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

                        cell = moveForward(cell, rightDirection);
                        backtrack(cell, rightDirection);
                        cell = moveBackwards(cell, rightDirection);

						//if (!automaticDecision)
							decisionHolder[decisionHolder.size() - 1].pop_back();
                    }
                }
            }
        }
    }

    postAction(cell, direction);

    TRACE(tracer.depth--);
}

void Analyzer::preAction(Cell* cell, int dir)
{
    depth++;
}

void Analyzer::postAction(Cell* cell, int dir)
{
    depth--;
}

void Analyzer::preOccupyAction(Cell* cell, int dir)
{
    // Note: touching obstacles does not really work
    // because we do not simulate the whole solution
    // (only partial)

    TRACE(cell->setDepth(tracer.depth));
    TRACE(cell->setLayer(tracer.layer));
}

void Analyzer::postOccupyAction(Cell* cell, int dir)
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

    if (cell->hasExits())
    {
        Component* comp = &level->getComponents()[cell->getComponentId()];
        StateMask mask = 1 << comp->getIndexByExitCell(cell);
        stateChangeStack[stateChangeStack.size() - 1] ^= mask;
    }
}

void Analyzer::preRestoreAction(Cell* cell, int dir)
{
    // Note: use reverse sequence! (working with stacks)

    const Cell* behind = cell->getNextCell(dir ^ 2);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* left  = cell->getNextCell(Left[dir]);
    if (behind->isTemporaryEnd()) // May happen to the first cell in a line
    {
        level->removeTemporaryEnd(behind);
        if (behind->getComponentId() == cell->getComponentId())
        {
            level->removeTemporaryEndsInCurrentComponent(behind);
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
    if (left->isTemporaryEnd())
    {
        level->removeTemporaryEnd(left);
        if (left->getComponentId() == cell->getComponentId())
        {
            level->removeTemporaryEndsInCurrentComponent(left);
        }
    }

    if (cell->hasExits())
    {
        Component* comp = &level->getComponents()[cell->getComponentId()];
        StateMask mask = 1 << comp->getIndexByExitCell(cell);
        stateChangeStack[stateChangeStack.size() - 1] ^= mask;
    }
}

void Analyzer::postRestoreAction(Cell* cell, int dir)
{
    TRACE(cell->setDepth(-1));
    TRACE(cell->setLayer(-1));
}

// TODO: consider checking only front direction
bool Analyzer::reachedFinalCell(Cell* cell, int dir) const
{
    // Very last free cell in the component
    const Component* comp = &level->getComponents()[cell->getComponentId()];
    if (comp->getOccupiedCount() + 1 == comp->getSize())
    {
        // TODO: consider moving this check to solutionFound()
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
//level->traceComponent();
        return false;
    }

    if (level->getTemporaryEnds().size() > 2)
    {
//level->traceComponent();
        return false;
    }

    return true;
}

void Analyzer::collectResults(const SolutionHead& head, const SolutionBody& body)
{
    SolutionRecord record(mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), head, body);
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

    int outExitIndex = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(outExit);

    assert((mustBeFreeStateMask.back() & (1 << outExitIndex)) == 0 && "Erroneous state mask!");

    const MustBeFreeMask prevMask = mustBeFreeStateMask.back();
    mustBeFreeStateMask.pop_back();
    mustBeFreeStateMask.push_back(prevMask | (1 << outExitIndex));

	SolutionBody body = { outExit->getX(), outExit->getY(), outExit->getDir(), mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), stateChangeStack.back(), decisionHolder.back() };
	// TODO: restore the line above
	//SolutionBody body = { outExit->getX(), outExit->getY(), outExit->getDir(), mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), stateChangeStack.back(), decisionHolder.back() + "|" };
    const SolutionHead& head = previousHead.back();

    collectResults(head, body);

    analyzeComponent(level->getComponents()[getComponentCurrentIndex()]);

    uncollectResults();

    mustBeFreeStateMask.pop_back();
    mustBeFreeStateMask.push_back(prevMask);
}

void Analyzer::solutionFound(Cell* cell, int dir)
{
    // Very last free cell
    Component& comp = level->getComponents()[cell->getComponentId()];
    if (comp.getOccupiedCount() + 1 == comp.getSize()) // Ending solution
    {
        // Occupy and finish
        Cell* ncell = moveForward(cell, dir);

        MustBeFreeMask mustBeFreeMask = mustBeFreeStateMask.back();
        if (cell->hasExit(dir))
        {
            mustBeFreeMask |= 1 << comp.getIndexByExit(cell->getExit(dir));
        }

		SolutionBody body = { cell->getX(), cell->getY(), dir, mustBeBlockedStateMask.back(), mustBeFreeMask, stateChangeStack.back(), decisionHolder.back() };
		// TODO: restore the line above
		//SolutionBody body = { cell->getX(), cell->getY(), dir, mustBeBlockedStateMask.back(), mustBeFreeMask, stateChangeStack.back(), decisionHolder.back() + "/"};
		
        const SolutionHead& head = previousHead.back();

        // TODO: consider updating masks here
        //int prevMask = previousStateMask.back();

        //int  leftDirection = Left[dir];
        //int rightDirection = Right[dir];
        //if (cell->hasExit(leftDirection) && cell->getNextCell(leftDirection)->isFree()) // exit on the left
        //{
        //    int newStateMask = prevMask;
        //    previousStateMask.pop_back();

        //    int exitIndex = comp.getIndexByExit(cell->getExit(leftDirection));
        //    assert((prevMask & (1 << exitIndex)) == 0);
        //    newStateMask |= 1 << exitIndex;
        //    previousStateMask.push_back(newStateMask);
        //}
        //if (cell->hasExit(rightDirection) && cell->getNextCell(rightDirection)->isFree()) // exit on the right
        //{
        //    int newStateMask = prevMask;
        //    previousStateMask.pop_back();

        //    int exitIndex = comp.getIndexByExit(cell->getExit(rightDirection));
        //    assert((prevMask & (1 << exitIndex)) == 0);
        //    newStateMask |= 1 << exitIndex;
        //    previousStateMask.push_back(newStateMask);
        //}

        collectResults(head, body);
        analyzeComponent(comp);
        uncollectResults();

        /*{
            previousStateMask.pop_back();
            previousStateMask.push_back(prevMask);
        }*/

        cell = moveBackwards(ncell, dir);
    }

    // Otherwise assuming cell->hasExits()
    if (cell->getNextCell(dir)->isFree() == false) // May stumble upon itself
    {
        // Try left / right
        int leftDirection = Left[dir];
		int rightDirection = Right[dir];
        if (cell->getNextCell(leftDirection)->isFree())
        {
			/*bool automaticDecision = !cell->getNextCell(rightDirection)->isFree();
			if (!automaticDecision)*/
				decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);
			//exitHolder[exitHolder.size() - 1].push_back(cell->getExit(leftDirection));

            if (cell->hasExit(leftDirection))
            {
                Cell* ncell = moveForward(cell, leftDirection);
                proceedAnalyzing(cell, leftDirection);
                moveBackwards(ncell, leftDirection);
            }
            else
            {
                // continue: turn left
                TRACE(tracer.depth--); // TODO: rethink ugly hack, because backtrack() is called a few extra times
                                       // What about moving preAction() trace code to moveForward()?
                backtrack(cell, leftDirection);
                TRACE(tracer.depth++);
            }

			//if (!automaticDecision)
				decisionHolder[decisionHolder.size() - 1].pop_back();
        }
            
        if (cell->getNextCell(rightDirection)->isFree())
        {
			//bool automaticDecision = !cell->getNextCell(leftDirection)->isFree();
			//if (!automaticDecision)
				decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);
				//decisionHolder[decisionHolder.size() - 1].push_back(tolower(Direction[rightDirection])); // TODO: restore this

            if (cell->hasExit(rightDirection))
            {
                Cell* ncell = moveForward(cell, rightDirection);
                proceedAnalyzing(cell, rightDirection);
                moveBackwards(ncell, rightDirection);
            }
            else
            {
                // continue: turn right
                TRACE(tracer.depth--);
                backtrack(cell, rightDirection);
                TRACE(tracer.depth++);
            }

			//if (!automaticDecision)
				decisionHolder[decisionHolder.size() - 1].pop_back();
        }
    }
    else
    {
        if (cell->hasExit(dir))
        {
            if (cell->getNextCell(dir)->isFree())
            {
                Cell* ncell = moveForward(cell, dir);
                proceedAnalyzing(cell, dir);
                moveBackwards(ncell, dir);
            }

            // Consider case: cell ahead has been occupied somewhen during Analysis
            if (cell->getNextCell(dir)->isFree())
            {
                cell->getNextCell(dir)->setFree(false); // Otherwise loops infinitely (turning left,right,left,...)

                int exitIndex = level->getComponents()[getComponentCurrentIndex()].getIndexByExit(cell->getExit(dir));
                const int prevBlockedMask = mustBeBlockedStateMask.back();
                mustBeBlockedStateMask.pop_back();
                assert(((1 << exitIndex) & prevBlockedMask) == 0 && "Erroneous state mask!");
                int newMask = prevBlockedMask | (1 << exitIndex);
                mustBeBlockedStateMask.push_back(newMask);

                int leftDirection = Left[dir];
                int rightDirection = Right[dir];

                if (cell->getNextCell(leftDirection)->isFree())
                {
					//bool automaticDecision = !cell->getNextCell(rightDirection)->isFree();
					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

                    TRACE(tracer.depth--);
                    backtrack(cell, leftDirection);
                    TRACE(tracer.depth++);

					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].pop_back();
                }
                if (cell->getNextCell(rightDirection)->isFree())
                {
					//bool automaticDecision = !cell->getNextCell(leftDirection)->isFree();
					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

                    TRACE(tracer.depth--);
                    backtrack(cell, rightDirection);
                    TRACE(tracer.depth++);

					//if (!automaticDecision)
						decisionHolder[decisionHolder.size() - 1].pop_back();
                }

                mustBeBlockedStateMask.pop_back();
                mustBeBlockedStateMask.push_back(prevBlockedMask);

                cell->getNextCell(dir)->setFree(true);
            }
        }
    }
}
