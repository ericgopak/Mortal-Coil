#pragma once

#include "Level.h"

typedef std::vector<std::vector<SolutionRecord>> SolutionList;
typedef std::map<int, SolutionList> SolutionListMap;
typedef std::map<int, SolutionTree> SolutionMap;

typedef std::pair<int, int> EndPoints;

class Deducer
{
    Level* level;

public:

    Deducer(Level* level);

    std::vector<EndPoints> deduceSolutions();
    bool deduceSolution(int componentFirst, int componentLast);

    bool checkCompatibility(int componentID, SolutionListMap& validSolutions, bool& solutionsUpdated) const;
};