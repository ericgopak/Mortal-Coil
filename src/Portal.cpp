#include "Common.h"
#include "Portal.h"

Portal::Portal()
    : directed(false)
    , firstGate(NULL)
    , lastGate(NULL)
{
}

bool Portal::isDirected() const
{
    return directed;
}

const Exit* Portal::getFirstGate() const
{
    return firstGate;
}

const Exit* Portal::getLastGate() const
{
    return lastGate;
}

//const PortalData& Portal::getGates() const
//{
//    return gates;
//}

//void Portal::addGate(const Exit* exit)
//{
//    gates.push_back(exit);
//}

void Portal::setDirected(bool directed)
{
    this->directed = directed;
}

void Portal::setFirstGate(const Exit* fromExit)
{
    firstGate = fromExit;
}

void Portal::setLastGate(const Exit* toExit)
{
    lastGate = toExit;
}
