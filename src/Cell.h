#pragma once

#include "Common.h"
#include "Portal.h"

//#define MAX_CAN_TOUCH  4 // Max # of components one cell can touch
#define MAX_NEXT_TOUCH 8 // Max # of neighbouring cells touching the same component

class Cell;
class Exit;
typedef std::map<int, std::set<const Cell*> > NextTouchMap;
class Obstacle;

class Cell
{
    enum CellType
    {
        FreeCell,
        Obstacle
    };

    CellType type;
    int x, y;
    bool free;
    int obstacleId; // Index of an obstacle cell belongs to. 0 for the 'border'
    int componentId; // Index of the component the cell belongs to
    int exitMask;

    std::vector<Exit> exits;
    const Exit* exitsByDirection[4];

    Cell* nextCell[8]; // Pointers to neighbouring cells (nextCell[dir])
    int nextMask; // Mask of free neighbours

    NextTouchMap neighboursTouchingSameObstacle;

    //Portal* portals[4]; // Portals in all 4 directions (max 2 possible, only 1 allowed!)
    const Portal* portal;

    TRACE(
        int depth;
        int layer;
    );

public:

    Cell(int x = -1, int y = -1);
    // TODO: Rule of thumb
    ~Cell();

    bool isObstacle() const;
    bool isFree() const;
    bool isOccupied() const;
    
    bool isTemporaryEnd() const; // if exactly one neighbour is free
    bool isPit() const;
    bool isThrough() const;
    bool hasExits() const;

    bool hasExit(int dir) const;
    const Exit* getExit(int dir) const;
    Cell* getNextCell(int dir) const;

    const NextTouchMap& getNeighboursTouchingSameObstacle() const;

    int getX() const;
    int getY() const;
    int getObstacleId() const;
    int getComponentId() const;
    int getNextMask() const;
    const std::vector<Exit>& getExits() const;

    //const Portal** getPortals() const;
    const Portal* getPortal() const;

    void addExit(const Exit& exit);
    void setOpposingExit(const Exit* exit, const Exit* opposingExit);
    void addNextTouch(int obstacleId, const Cell* cell);

    void setType(bool obstacle);
    void setXY(int x, int y);
    void setComponentId(int id);
    void setObstacleId(int id);
    void setFree(bool free);
    void setNextMask(int mask);
    void setNextCell(int dir, Cell* ncell);

    void setPortal(const Portal* p);
    
#ifdef TRACE_SOLUTIONS
    int getDepth() const;
    int getLayer() const;
    void setDepth(int depth);
    void setLayer(int layer);
#endif
};

class Exit
{
    int dir;
    Cell* hostCell;
    const Exit* opposingExit;

public:
    Exit(Cell* cell, int dir);

    bool operator < (const Exit &e) const;
    bool operator == (const Exit &e) const;

    int getX() const;
    int getY() const;
    int getDir() const;
    const Cell* getHostCell() const;
    const Exit* getOpposingExit() const;

    void setOpposingExit(const Exit* opposingExit);
};
