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

//static SolutionBody getBodyFromExits(const Exit* e1, const Exit* e2)
//{
//    SolutionBody body = {e2->getX(), e2->getY(), e2->getDir()};
//
//    if ((e1->getDir() ^ 2) != e2->getDir())
//    {
//        body.solution += Direction[e2->getDir()];
//    }
//
//    return body;
//}

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

            // Through solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];
                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                std::string decision;
                if ((e1->getDir() ^ 2) != e2->getDir())
                {
                    decision = Direction[e2->getDir()];
                }
                // TODO: consider mask ~((1 << i) | (1 << j))
                MustBeBlockedMask mustBeBlockedMask = 1 << i; // TODO: could also be 0 -> unify with Solver & Analyzer
                MustBeFreeMask mustBeFreeMask = 1 << j;

                //SolutionBody body = {e2->getX(), e2->getY(), e2->getDir(), decision};
                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
                SolutionBody body = {e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision};

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getSolutions()->addSolution(v, 0, 0);
            }

            // Starting solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];

                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                std::string decision;
                decision += Direction[e2->getDir()];

                MustBeBlockedMask mustBeBlockedMask = 0;
                MustBeFreeMask mustBeFreeMask = (1 << j) | (1 << i); // This distinguishes starting from through solutions

                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
                SolutionBody body = {e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision};

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getSolutions()->addSolution(v, 1, 0); // Is starting, not ending
            }

            // Ending solutions
            {
                const Exit* e1 = component->getExits()[i];
                const Exit* e2 = component->getExits()[j];

                SolutionHead head = {e1->getX(), e1->getY(), e1->getDir() ^ 2};
                std::string decision; // Do nothing

                MustBeBlockedMask mustBeBlockedMask = (1 << component->getExits().size()) - 1; // ~0
                MustBeFreeMask mustBeFreeMask = 0;

                StateMask stateChange = (1 << component->getExitCells().size()) - 1; // ~0. All exit cells are getting blocked
                SolutionBody body = {e2->getX(), e2->getY(), e2->getDir(), mustBeBlockedMask, mustBeFreeMask, stateChange, decision};

                SolutionRecord solution(mustBeBlockedMask, mustBeFreeMask, head, body);

                std::vector<SolutionRecord> v(1, solution);
                component->getSolutions()->addSolution(v, 0, 1); // Is starting, not ending
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

                SolutionHead h1 = {e->getX(), e->getY(), e->getDir()};
                SolutionHead h2 = {e->getX(), e->getY(), e->getDir() ^ 2};

                MustBeBlockedMask mustBeBlockedMask1 = 0;
                MustBeFreeMask mustBeFreeMask1 = 1 << 0;
                MustBeBlockedMask mustBeBlockedMask2 = 1 << 0;
                MustBeFreeMask mustBeFreeMask2 = 0;

                StateMask stateChange1 = 1 << 0; // ~0
                StateMask stateChange2 = 1 << 0; // ~0

                SolutionBody b1 = {e->getX(), e->getY(), e->getDir()    , mustBeBlockedMask1, mustBeFreeMask1, stateChange1, decision1};
                SolutionBody b2 = {e->getX(), e->getY(), e->getDir() ^ 2, mustBeBlockedMask2, mustBeFreeMask2, stateChange2, decision2};

                // Starting solution
                SolutionRecord solution1(mustBeBlockedMask1, mustBeFreeMask1, h1, b1); // Exit must be free
                // Ending solution
                SolutionRecord solution2(mustBeBlockedMask2, mustBeFreeMask2, h2, b2); // Exit must have been occupied

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getSolutions()->addSolution(v1, 1, 0);
                component.getSolutions()->addSolution(v2, 0, 1);
            }
            else if (exits.size() == 2)
            {
                generateAllSimpleSolutions(&component);

                /*const Exit* e1 = exits[0];
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

                SolutionRecord solution1(, h1, b1);
                SolutionRecord solution2(, h2, b2);

                std::vector<SolutionRecord> v1(1, solution1);
                std::vector<SolutionRecord> v2(1, solution2);

                component.getSolutions()->addSolution(v1, 0, 0);
                component.getSolutions()->addSolution(v2, 0, 0);*/
            }
            else if (exits.size() == 3) // Single-cell component with 3 exits
            {
//                assert(false && "Should not ever happen!");

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
// TODO: confirm - is the problem with mustBeFreeMask?
    // Exit in front and it must be free
    if (cell->hasExit(endDir) && (mustBeFreeMask & (1 << comp->getIndexByExit(cell->getExit(endDir)))) != 0)
    {
        isEnding = false;
    }

    return isEnding;
}

//std::vector<const Exit*> Analyzer::getExitSequenceFromSolution(const std::vector<SolutionRecord>& solution) const
std::vector<UniqueSolutionFragment> Analyzer::getUniqueSolutionFragments(const std::vector<SolutionRecord>& solution) const
{
    // TODO: assign masks as well!!!
    const size_t n = solution.size();

    std::vector<UniqueSolutionFragment> res(n);

    if (n == 0)
    {
        return res;
    }

    // First fragment
    std::get<0>(res[0]) = std::get<0>(solution[0]); // mustBeBlocked
    std::get<1>(res[0]) = std::get<1>(solution[0]); // mustBeFree

    if (solutionIsStarting(solution))
    {
        std::get<2>(res[0]) = NULL;
    }
    else
    {
        const SolutionHead& head = std::get<2>(solution[0]);
        std::get<2>(res[0]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
    }

    if (n > 1)
    {
        const SolutionBody& body = std::get<3>(solution[0]);
        std::get<3>(res[0]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    // Middle fragments
    for (size_t i = 1; i < solution.size() - 1; i++)
    {
        std::get<0>(res[i]) = std::get<0>(solution[i]);
        std::get<1>(res[i]) = std::get<1>(solution[i]);

        const SolutionHead& head = std::get<2>(solution[i]);
        const SolutionBody& body = std::get<3>(solution[i]);
        std::get<2>(res[i]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
        std::get<3>(res[i]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    if (n > 1)
    {
        // Penultimate fragment
        const SolutionHead& head = std::get<2>(solution[solution.size() - 1]);
        std::get<2>(res[n - 1]) = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
    }

    // Ultimate fragment
    if (n > 1) // Just don't repeat the code for the first fragment
    {
        std::get<0>(res[n - 1]) = std::get<0>(solution[n - 1]);
        std::get<1>(res[n - 1]) = std::get<1>(solution[n - 1]);
    }

    if (solutionIsEnding(solution))
    {
        std::get<3>(res[n - 1]) = NULL;
    }
    else
    {
        const SolutionBody& body = std::get<3>(solution[solution.size() - 1]);
        std::get<3>(res[n - 1]) = level->getCell(body.endY, body.endX)->getExit(body.endDir);
    }

    return res;
}

void Analyzer::analyzeComponent(Component& component)
{
    if (component.getOccupiedCount() == component.getSize())
    {
        if (solutionRecordHolder.size() > 0)
        {
            int solutionNumber = component.getSolutionCount() + 1;
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
//int x = std::get<2>(solutionRecordHolder[0]).startX;
//int y = std::get<2>(solutionRecordHolder[0]).startY;
////if ((*component.getCells().begin())->getComponentId() == 25)
//if (solutionRecordHolder.size() == 3 && x == 16 && y == 6)
//{
//    if (std::get<2>(solutionRecordHolder[1]).startX == 13
//     && std::get<2>(solutionRecordHolder[1]).startY == 9)
//    {
//level->traceComponent();
////int dir = std::get<2>(solutionRecordHolder[0]).startDir;
//int bp = 0;
//    }
//}
            bool isStarting = solutionIsStarting(solutionRecordHolder);
            bool isEnding   = solutionIsEnding(solutionRecordHolder);

            auto newSolution = getUniqueSolutionFragments(solutionRecordHolder);

            bool unique = true;
            FOREACH_CONST(uniqueSolutions, sol)
            {
                auto solution = *sol;
                if (solution.size() != solutionRecordHolder.size())
                {
                    continue;
                }

                auto oldSolution = getUniqueSolutionFragments(solution);

                if (newSolution == oldSolution)
                {
                    unique = false;
                    break;
                }
            }

            if (unique == false)
            {
                int bp = 0;
#ifdef TRACE_STATISTICS
                Debug::similarSolutionsCounter++;
#endif
            }
            else
            {
//level->traceComponent();
                uniqueSolutions.insert(solutionRecordHolder);
                component.getSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
            }

            //component.getSolutions()->addSolution(solutionRecordHolder, isStarting, isEnding);
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
        stateChangeStack.push_back(0);

        if (solutionRecordHolder.size() == 0)
        {
            FOREACH(component.getCells(), c)
            {
                Cell* cell = const_cast<Cell*>(*c); // UGLY HACK!!! TODO: Reconsider!!!

                for (int dir = 0; dir < 4; dir++)
                {
                    if (cell->getNextCell(dir)->isObstacle() == false
                        //&& cell->getComponentId() == cell->getNextCell(dir)->getComponentId() TODO: what was this condition for??? == "If not an exit"?
                        )
                    {
                        assert(cell->getNextCell(dir)->isFree());
                        //assert(stateMask == 0);

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

                            //previousStateMask.push_back(stateMask | (1 << exitIndex));
                            //mustBeFreeStateMask.push_back(1 << exitIndex);
                            //mustBeFreeStateMask.push_back(0);
                            mustBeBlockedStateMask.push_back(0);

                            // TODO: WHY TO CONVERT TO OBSTACLE? IT MAKES NO SENSE!

                            //prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                            //prevCell->setFree(false);
                            backtrack(cell, dir);
                            /*prevCell->setType(false);
                            prevCell->setFree(true);*/
                            //previousStateMask.pop_back();
                            mustBeFreeStateMask.pop_back();
                            mustBeBlockedStateMask.pop_back();
                            decisionHolder[decisionHolder.size() - 1].pop_back();
                        }
                        else
                        // Starting solution
                        //if (prevCell->isObstacle() || level->getComponents()[prevCell->getComponentId()].getSize() != 1)
                        if (prevCell->isObstacle() == false && level->getComponents()[prevCell->getComponentId()].getSize() == 1) // Special case: there's a narrow exit behind
                        {
                            // There is no point in starting from the exitCell with 1-sized component behind

                            //decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);
                            ////TODO: remove this 'if' ?
                            //if (cell->hasExit(dir ^ 2))
                            //{
                            //    int exitIndex = level->getComponents()[cell->getComponentId()].getIndexByExit(cell->getExit(dir ^ 2));
                            //    mustBeFreeStateMask.push_back(1 << exitIndex);
                            //}
                            //else
                            //{
                            //    mustBeFreeStateMask.push_back(0);
                            //}
                            ////previousStateMask.push_back(stateMask);
                            ////mustBeFreeStateMask.push_back(0);
                            ////mustBeFreeStateMask.push_back(0);
                            //mustBeBlockedStateMask.push_back(0);
                            //backtrack(cell, dir);
                            //// TODO: consider pushing this cell onto temporaryEnds!
                            ////previousStateMask.pop_back();
                            //mustBeFreeStateMask.pop_back();
                            //mustBeBlockedStateMask.pop_back();

                            //decisionHolder[decisionHolder.size() - 1].pop_back();
                        }
                        else // Normal cases
                        {
                            decisionHolder[decisionHolder.size() - 1].push_back(Direction[dir]);

                            mustBeFreeStateMask.push_back(0);
                            //previousStateMask.push_back(stateMask);
                            //mustBeFreeStateMask.push_back(0);
                            //mustBeFreeStateMask.push_back(0);
                            mustBeBlockedStateMask.push_back(0);
                            backtrack(cell, dir);
                            // TODO: consider pushing this cell onto temporaryEnds!
                            //previousStateMask.pop_back();
                            mustBeFreeStateMask.pop_back();
                            mustBeBlockedStateMask.pop_back();

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
                //if (((1 << i) & stateMask) == 0) // if exit is (supposedly) free // TODO: use another mask
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
                        //previousStateMask.push_back(stateMask | (1 << i)); // Through solution: mask the exit out
                        mustBeFreeStateMask.push_back(0);
                        // TODO: test this
                        //mustBeBlockedStateMask.push_back(0);
                        mustBeBlockedStateMask.push_back(1 << i);

                        prevCell->setType(true); // Convert to obstacle in order not to count it as a (potential) temporary end
                        prevCell->setFree(false);
//printf("BEFORE:\n");
//level->traceComponent();
                        backtrack(cell, dir ^ 2);
//printf("AFTER:\n");
//level->traceComponent();
                        prevCell->setFree(true);
                        prevCell->setType(false);

                        previousHead.pop_back();
                        //previousStateMask.pop_back();
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
//level->traceComponent();
        if (left->getComponentId() == cell->getComponentId())
        {
            level->addTemporaryEndsInCurrentComponent(left);
        }
    }
    if (right->isTemporaryEnd())
    {
        level->addTemporaryEnd(right);
//level->traceComponent();
        if (right->getComponentId() == cell->getComponentId())
        {
            level->addTemporaryEndsInCurrentComponent(right);
        }
    }
    if (behind->isTemporaryEnd()) // May happen to the first cell in a line
    {
        level->addTemporaryEnd(behind);
//level->traceComponent();
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
    //SolutionRecord record(previousStateMask.back(), head, body);
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

    //assert((previousStateMask.back() & (1 << outExitIndex)) == 0 && "Erroneous state mask!");
    assert((mustBeFreeStateMask.back() & (1 << outExitIndex)) == 0 && "Erroneous state mask!");

    const MustBeFreeMask prevMask = mustBeFreeStateMask.back();
    mustBeFreeStateMask.pop_back();
    mustBeFreeStateMask.push_back(prevMask | (1 << outExitIndex));

    //SolutionBody body = {outExit->getX(), outExit->getY(), outExit->getDir(), decisionHolder.back()};
    //SolutionBody body = {outExit->getX(), outExit->getY(), outExit->getDir(), mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), decisionHolder.back()};
    SolutionBody body = {outExit->getX(), outExit->getY(), outExit->getDir(), mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), stateChangeStack.back(), decisionHolder.back()};
    const SolutionHead& head = previousHead.back();

    collectResults(head, body);

    //int newStateMask = previousStateMask.back() | (1 << outExitIndex);
    //int newStateMask = previousStateMask.back() | (1 << outExitIndex);
    //cell = moveForward(cell, dir);
//level->traceComponent();
    //analyzeComponent(level->getComponents()[getComponentCurrentIndex()], newStateMask);
    analyzeComponent(level->getComponents()[getComponentCurrentIndex()]);
    //cell = moveBackwards(cell, dir);

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

        // TODO: update mustBeFree mask
        MustBeFreeMask mustBeFreeMask = mustBeFreeStateMask.back();
        if (cell->hasExit(dir))
        {
            mustBeFreeMask |= 1 << comp.getIndexByExit(cell->getExit(dir));
        }

        //SolutionBody body = {cell->getX(), cell->getY(), dir, mustBeBlockedStateMask.back(), mustBeFreeStateMask.back(), decisionHolder.back()};
        SolutionBody body = {cell->getX(), cell->getY(), dir, mustBeBlockedStateMask.back(), mustBeFreeMask, stateChangeStack.back(), decisionHolder.back()};
        const SolutionHead& head = previousHead.back();

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
        //analyzeComponent(comp, previousStateMask.back()); // TODO: replace mask with -1
        analyzeComponent(comp);
        uncollectResults();

        /*{
            previousStateMask.pop_back();
            previousStateMask.push_back(prevMask);
        }*/

        cell = moveBackwards(ncell, dir);
    }

    // Otherwise assuming cell->hasExits()
    //if (cell->getNextCell(dir)->isObstacle())
    if (cell->getNextCell(dir)->isFree() == false) // May stumble upon itself
    {
        // Try left / right
        int leftDirection = Left[dir];
        if (cell->getNextCell(leftDirection)->isFree())
        {
            decisionHolder[decisionHolder.size() - 1].push_back(Direction[leftDirection]);

            if (cell->hasExit(leftDirection))
            {
                Cell* ncell = moveForward(cell, leftDirection);
//level->traceComponent();
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

            decisionHolder[decisionHolder.size() - 1].pop_back();
        }
            
        int rightDirection = Right[dir];
        if (cell->getNextCell(rightDirection)->isFree())
        {
            decisionHolder[decisionHolder.size() - 1].push_back(Direction[rightDirection]);

            if (cell->hasExit(rightDirection))
            {
                Cell* ncell = moveForward(cell, rightDirection);
//level->traceComponent();
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

            decisionHolder[decisionHolder.size() - 1].pop_back();
        }
    }
    else
    {
        if (cell->hasExit(dir))
        {
//level->traceComponent();
            // TODO: this condition seems to be true overall
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
                //const int prevMask = previousStateMask.back();
                const int prevBlockedMask = mustBeBlockedStateMask.back();
                //previousStateMask.pop_back();
                mustBeBlockedStateMask.pop_back();
                //assert(((1 << exitIndex) & prevMask) == 0 && "Erroneous state mask!");
                assert(((1 << exitIndex) & prevBlockedMask) == 0 && "Erroneous state mask!");
                //int newMask = prevMask | (1 << exitIndex);
                int newMask = prevBlockedMask | (1 << exitIndex);
                //previousStateMask.push_back(newMask);
                mustBeBlockedStateMask.push_back(newMask);

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

                //previousStateMask.pop_back();
                mustBeBlockedStateMask.pop_back();
                //previousStateMask.push_back(prevMask);
                mustBeBlockedStateMask.push_back(prevBlockedMask);

                cell->getNextCell(dir)->setFree(true);
            }
        }
    }
}
