#pragma once

#include "Level.h"

//typedef std::pair<int, int> EndPoints;
class EndPoints : public std::pair < int, int >
{
public:

    EndPoints(int a, int b);

    bool operator < (const EndPoints& ep) const;
};


class Deducer
{
    Level* level;

public:

    Deducer(Level* level);

    bool deduceSolutions(std::set<EndPoints>& endpointHistory, EndPoints& endpoints, SolutionListMap& cleanerSolutions, int comp1, int comp2);
    bool deduceEndpoints(int componentFirst, int componentLast, SolutionListMap& cleanerSolutions, int& componentCandidate);

    bool checkCompatibility(int componentID, SolutionListMap& validSolutions, bool& solutionsUpdated, int& componentCandidate) const;

    void reduceSolutions(SolutionListMap& reducedSolutions);
};