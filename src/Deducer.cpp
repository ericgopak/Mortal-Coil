#include "Deducer.h"
#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"


Deducer::Deducer(Level* level)
    : level(level)
{
}

std::vector<EndPoints> Deducer::deduceSolutions()
{
    std::vector<EndPoints> endpoints;

    for (int i = 0; i < level->getComponentCount(); i++)
    {
        for (int j = 0; j < level->getComponentCount(); j++)
        {
            if (i == j) continue;

            if (level->getComponents()[i].getSize() == 1 && level->getComponents()[j].getSize() == 1)
            {
                continue; // Avoid starting in 1-cell components
            }

            if (deduceSolution(i, j))
            {
                endpoints.push_back(EndPoints(i, j));
            }
        }
    }

    return endpoints;
}

bool Deducer::deduceSolution(int componentFirst, int componentLast)
{
    //Colorer::print<GREEN>("Starting from %d -> ending in %d\n", componentFirst, componentLast);
    /*level->traceComponent(componentFirst);
    level->traceComponent(componentLast);*/

    std::map<int, SolutionTree> validSolutions;

    for (int i = 0; i < level->getComponentCount(); i++)
    {
        if (i == componentFirst)
        {
            validSolutions[i] = *level->getComponents()[i].getStartingSolutions();
        }
        else if (i == componentLast)
        {
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

    /*for (int i = 0; i < level->getComponentCount(); i++)
        Colorer::print<GREEN>("Solutions BEFORE: %d\n", solutionLists[i].size());*/
    
    bool ok = true;
    bool solutionsUpdated = true;
    while (ok && solutionsUpdated)
    {
        solutionsUpdated = false;

        for (int i = 0; i < level->getComponentCount(); i++)
        {
            if (checkCompatibility(i, solutionLists, solutionsUpdated) == false)
            {
                ok = false;
                break;
            }
        }
    }

    /*for (int i = 0; i < level->getComponentCount(); i++)
        Colorer::print<GREEN>("Solutions AFTER: %d\n", solutionLists[i].size());*/

    for (int i = 0; i < level->getComponentCount(); i++)
    {
        if (solutionLists[i].size() == 0)
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        Colorer::print<GREEN>("<<< Oh YEAH!!! >>>  %d -> %d\n", componentFirst, componentLast);
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
bool Deducer::checkCompatibility(int componentID, SolutionListMap& validSolutions, bool& solutionsUpdated) const
{
    auto& solutions = validSolutions[componentID];

    for (auto it = solutions.cbegin(); it != solutions.cend(); )
    {
        const auto& solution = *it;

        bool valid = true;

        for (int i = 0; i < (int)solution.size(); i++) {
            const SolutionRecord& record = solution[i];
            const SolutionHead& head = std::get<2>(record);
            const SolutionBody& body = std::get<3>(record);

            const Exit* inExit = level->getCell(head.startY, head.startX)->getExit(head.startDir ^ 2);
            const Exit* outExit = level->getCell(body.endY, body.endX)->getExit(body.endDir);

            if (inExit != NULL) // Not starting solution
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
                }
            }

            if (outExit != NULL) // Not ending solution
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
                }
            }
        }

        if (valid == false)
        {
            it = solutions.erase(it);
            solutionsUpdated = true;

            if (solutions.size() == 0)
            {
                //Colorer::print<RED>("CHECK THIS OUT: %d\n", componentID);
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
