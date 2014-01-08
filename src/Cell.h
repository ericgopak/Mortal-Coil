#pragma once

#include "Common.h"

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

protected:
    CellType type;
    int x, y;
    bool free;
    int obstacleId; // Index of an obstacle cell belongs to. 0 for the 'border'
    int componentId; // Index of the component the cell belongs to
    int exitMask;
    TRACE(
        int depth;
        int layer;
    );

    std::vector<Exit> exits;
    const Exit* exitsByDirection[4];

    Cell* nextCell[8]; // Pointers to neighbouring cells (nextCell[dir])
    int nextMask; // Mask of free neighbours

    NextTouchMap neighboursTouchingSameObstacle;
    //std::set< ::Obstacle*> touchingObstacles;

public:

    Cell(int x = -1, int y = -1);

    bool isObstacle() const;
    bool isFree() const;
    bool isOccupied() const;
    
    bool isTemporaryEnd() const; // if exactly one neighbour is free
    bool isPit() const;
    bool isThrough() const;
    bool hasExits() const;

    bool mayBeFirst() const; // TODO: seems obolete... remove!
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
    //std::set< ::Obstacle*>* getTouchingObstacles();

    void addExit(const Exit& exit);
    void setOpposingExit(const Exit* exit, const Exit* opposingExit);
    void addNextTouch(int obstacleId, const Cell* cell);
    //void addTouchingObstacle(::Obstacle* obst);

    void setType(bool obstacle);
    void setXY(int x, int y);
    void setComponentId(int id);
    void setObstacleId(int id);
    void setFree(bool free);
    void setNextMask(int mask);
    void setNextCell(int dir, Cell* ncell);
    
#ifdef TRACE_SOLUTIONS
    int getDepth() const;
    int getLayer() const;
    void setDepth(int depth);
    void setLayer(int layer);
#endif
};

//class Exit : public Cell
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


//class Exit : public Cell
//{
//    int dir;
//    const Exit* opposingExit;
//
//public:
//    Exit(const Cell* cell, int dir);
//
//    const Exit* getOpposingExit() const;
//    void setOpposingExit(const Exit* opposingExit);
//
//    bool operator < (const Exit &e) const;
//    bool operator == (const Exit &e) const;
//
//    int getDir() const;
//};
