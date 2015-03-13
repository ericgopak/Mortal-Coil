#include "Deducer.h"
#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"

EndPoints::EndPoints(int a, int b)
    : std::pair<int, int>(a, b)
{
}

bool EndPoints::operator < (const EndPoints& ep) const
{
    if (first != ep.first) return first < ep.first;
    return second < ep.second;
}

Deducer::Deducer(Level* level)
    : level(level)
{
}

bool Deducer::deduceSolutions(std::set<EndPoints>& endpointHistory, EndPoints& endpoints, SolutionListMap& cleanerSolutions, int comp1, int comp2)
{
    // TODO: rewrite

    //if (comp1 == -1) // No need to repeat once it's deduced
    {
        if (deduceEndpoints(-1, -1, cleanerSolutions, comp1))
        {
            Colorer::print<RED>("Not implemented: all components appear to be compatible!\n");
            return false;
        }
    }

    assert(comp1 != -1 && "Failed to deduce first candidate component!");

    auto s = level->getComponents()[comp1].getNeighbours();
    s.insert(comp1);

    for (auto i : s)
    {
        // comp2 may depend on the value of i
        if (deduceEndpoints(i, -1, cleanerSolutions, comp2))
        {
            Colorer::print<YELLOW>("Note#1: perhaps component %d is both starting and ending!\n", i);
            auto p = EndPoints(i, i);
            if (endpointHistory.find(p) == endpointHistory.end())
            {
                endpoints = p;
                return true;
            }
            continue;
        }

        assert(comp1 != -1 && "Failed to deduce second candidate component!");

        //if (s.find(comp2) == s.end())
        if (comp2 != comp1) // TODO: test
        {
            auto t = level->getComponents()[comp2].getNeighbours();
            t.insert(comp2);
            for (auto j : t)
            {
printf("1) (%d,%d)\n", i, j);
                int _ = -1;
                if (deduceEndpoints(i, j, cleanerSolutions, _))
                {
                    auto p = EndPoints(i, j);
                    if (endpointHistory.find(p) == endpointHistory.end())
                    {
                        endpoints = p;
                        return true;
                    }
                }
            }
        }

        if (deduceEndpoints(-1, i, cleanerSolutions, comp2))
        {
            Colorer::print<YELLOW>("Note#2: perhaps component %d is both starting and ending!\n", i);
            auto p = EndPoints(i, i);
            if (endpointHistory.find(p) == endpointHistory.end())
            {
                endpoints = p;
                return true;
            }
            continue;
        }

        if (s.find(comp2) == s.end())
        {
            auto t = level->getComponents()[comp2].getNeighbours();
            t.insert(comp2);
            for (auto j : t)
            {
printf("2) (%d,%d)\n", i, j);
                int _ = -1;
                if (deduceEndpoints(j, i, cleanerSolutions, _))
                {
                    auto p = EndPoints(j, i);
                    if (endpointHistory.find(p) == endpointHistory.end())
                    {
                        endpoints = p;
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool Deducer::deduceEndpoints(int componentFirst, int componentLast, SolutionListMap& cleanerSolutions, int& componentCandidate)
{
    //Colorer::print<GREEN>("Starting from %d -> ending in %d\n", componentFirst, componentLast);
    /*level->traceComponent(componentFirst);
    level->traceComponent(componentLast);*/

    componentCandidate = -1;

    std::map<int, SolutionTree> validSolutions;

    for (int i = 0; i < level->getComponentCount(); i++)
    {
        if (i == componentFirst)
        {
            //validSolutions[i] = *level->getComponents()[i].getNonEndingSolutions();
            validSolutions[i] = *level->getComponents()[i].getStartingSolutions();
        }
        else if (i == componentLast)
        {
            //validSolutions[i] = *level->getComponents()[i].getNonStartingSolutions();
            validSolutions[i] = *level->getComponents()[i].getEndingSolutions();
        }
        else
        {
            validSolutions[i] = *level->getComponents()[i].getThroughSolutions();
        }
    }

    SolutionListMap solutionLists;
    FOREACH_CONST(validSolutions, it)
    {
        solutionLists[it->first] = it->second.convertToRecords();
    }

    size_t total = 0;
    for (int i = 0; i < level->getComponentCount(); i++)
    {
        //Colorer::print<GREEN>("Solutions BEFORE: %3d -> %d\n", i, solutionLists[i].size());
        total += solutionLists[i].size();
    }
    //Colorer::print<GREEN>("Total Solutions BEFORE: %d\n", total);
    
    bool ok = true;
    bool solutionsUpdated = true;
    while (ok && solutionsUpdated)
    {
        solutionsUpdated = false;

        for (int i = 0; i < level->getComponentCount(); i++)
        {
            if (checkCompatibility(i, solutionLists, solutionsUpdated, componentCandidate) == false)
            {
                ok = false;
                break;
            }
        }
    }

    size_t total2 = 0;
    for (int i = 0; i < level->getComponentCount(); i++)
    {
        //Colorer::print<GREEN>("Solutions AFTER: %3d -> %d\n", i, solutionLists[i].size());
        total2 += solutionLists[i].size();
    }
    //Colorer::print<GREEN>("Total Solutions AFTER: %d\n", total2);

    for (int i = 0; i < level->getComponentCount(); i++)
    {
        if (solutionLists[i].size() == 0)
        {
            //Colorer::print<YELLOW>("CHECK THIS OUT: %d\n", i);
            //globalVarsSuck_PoC_specialComponent = i;

            //level->traceComponent(i);

            ok = false;
            break;
        }
    }

    if (ok)
    {
        Colorer::print<GREEN>("<<< Oh YEAH!!! >>>  %d -> %d\n", componentFirst, componentLast);

        cleanerSolutions = solutionLists;
        //cleanerSolutions = backup; // TODO: remove!

        /*level->traceComponent(componentFirst);
        level->traceComponent(componentLast);*/
        /*for (int i = 0; i < level->getComponentCount(); i++)
            Colorer::print<GREEN>("Solutions AFTER: %d\n", solutionLists[i].size());*/
        return true;
    }

    return false;
}

static std::set<const Exit*> getOutExits(Level* level, const SolutionList& solutions)
{
    std::set<const Exit*> exits;

    for (const auto& sol : solutions)
    {
        for (const auto& record : sol)
        {
            const auto& body = std::get<3>(record);
            const auto outExit = level->getCell(body.endY, body.endX)->getExit(body.endDir);
            exits.insert(outExit);
        }
    }

    return exits;
}

static std::set<const Exit*> getInExits(Level* level, const SolutionList& solutions)
{
    std::set<const Exit*> exits;

    for (const auto& sol : solutions)
    {
        for (const auto& record : sol)
        {
            const auto& head = std::get<2>(record);
            const auto inExit = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
            exits.insert(inExit);
        }
    }

    return exits;
}

// Remove solutions from validSolutions[i] which are incompatible with neighbours
bool Deducer::checkCompatibility(int componentID, SolutionListMap& validSolutions, bool& solutionsUpdated, int& componentCandidate) const
{
    auto& solutions = validSolutions[componentID];

    for (auto it = solutions.cbegin(); it != solutions.cend(); )
    {
        const auto& solution = *it;

        bool valid = true;

        for (int i = 0; i < (int)solution.size(); i++) {
            const SolutionRecord& record = solution[i];
            const auto& mustBeBlockedMask = std::get<0>(record);
            const auto& mustBeFreeMask = std::get<1>(record);
            const auto& head = std::get<2>(record);
            const auto& body = std::get<3>(record);

            const Exit* inExit = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
            const Exit* outExit = level->getCell(body.endY, body.endX)->getExit(body.endDir);

            const auto& comp = &level->getComponents()[componentID];
            bool isStarting = inExit == NULL || (inExit != NULL && mustBeBlockedMask == 0);
            bool isEnding = outExit == NULL || (outExit != NULL && (!((1 << comp->getIndexByExit(outExit)) & mustBeFreeMask)));

            if (!isStarting) // Not starting solution
            {
                // OutExits of the next component - our inExit should be reachable from any of them
                //auto allOutExits = validSolutions[inExit->getOpposingExit()->getHostCell()->getComponentId()].getAllOutExits(level);
                auto allOutExits = getOutExits(level, validSolutions[inExit->getOpposingExit()->getHostCell()->getComponentId()]);

                if (std::find(allOutExits.begin(), allOutExits.end(), inExit->getOpposingExit()) == allOutExits.end())
                {
//Colorer::print<RED>("Incompatible due to inExit(%d,%d,%d)\n", inExit->getX(), inExit->getY(), inExit->getDir());
//level->traceComponent(componentID);
//level->traceComponent(inExit->getOpposingExit()->getHostCell()->getComponentId());
                    valid = false;
                    break;
                }
            }

            if (!isEnding) // Not ending solution
            {
                // InExits of the next component - some of them should be reachable from our outExit
                //auto allInExits = validSolutions[outExit->getOpposingExit()->getHostCell()->getComponentId()].getAllInExits(level);
                auto allInExits = getInExits(level, validSolutions[outExit->getOpposingExit()->getHostCell()->getComponentId()]);

                if (std::find(allInExits.begin(), allInExits.end(), outExit->getOpposingExit()) == allInExits.end())
                {
//Colorer::print<RED>("Incompatible due to outExit(%d,%d,%d)\n", outExit->getX(), outExit->getY(), outExit->getDir());
//level->traceComponent(componentID);
//level->traceComponent(outExit->getOpposingExit()->getHostCell()->getComponentId());
                    valid = false;
                    break;
                }
            }
        }

        if (valid == false)
        {
            it = solutions.erase(it);
            solutionsUpdated = true;

            if (solutions.size() == 0)
            {
                /*Colorer::print<RED>("CHECK THIS OUT: %d\n", componentID);
                Colorer::print<WHITE>("Neighbours:");
                for (int neighbour : level->getComponents()[componentID].getNeighbours())
                {
                    Colorer::print<RED>(" %d", neighbour);
                }
                Colorer::print<RED>("\n");*/

                componentCandidate = componentID;

                //level->traceComponent(componentID);
                return false;
            }
        }
        else
        {
            it++;
        }
    }

    return true;
}
