#pragma once

class Exit;

// TODO: how to deal with bidirectional portals?

//typedef std::deque<const Exit*> PortalData;

class Portal
{
    bool directed;

    const Exit* firstGate;
    const Exit* lastGate;
    //PortalData gates; // Sequence of exits: if enter the first one, then transfer to the last one

    std::string solution; // Sequence of decisions

public:

    Portal();

    bool isDirected() const;
    const Exit* getFirstGate() const;
    const Exit* getLastGate() const;

    //void addGate(const Exit* exit);
	void setDirected(bool directed);
    void setFirstGate(const Exit* fromExit);
    void setLastGate(const Exit* toExit);
};
