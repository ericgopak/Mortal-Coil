#pragma once

#define MAX_CAN_TOUCH  4 // Max # of components one cell can touch
#define MAX_NEXT_TOUCH 8 // Max # of neighbouring cells touching the same component

class Cell;
class Exit;

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
    int mark; // ID of a corresponding path segment

    //std::set<Exit> exits;
    std::vector<Exit> exits;
    const Exit* exitsByDirection[4];

    Cell* nextCell[4]; // Pointers to neighbouring cells (nextCell[dir])

    int nextMask; // Mask of free neighbours
    //bool is_exit; // True if this cell is an 'exit' for some component

public:

    int Touch[MAX_CAN_TOUCH]; // List of indeces of components this cell touches // unique indeces!!!
    int touch; // Amount of touched components
    Cell* NextTouch[MAX_CAN_TOUCH][MAX_NEXT_TOUCH]; // Neighbouring cells that touch (NextTouch[touch][nexttouch[touch]])
    int nexttouch[MAX_CAN_TOUCH]; // Number of nearby cells also touching Touch[i] component

    Cell(int x = -1, int y = -1);
    ~Cell();

    bool isObstacle() const;
    bool isFree() const;
    bool isOccupied() const;
    
    bool isTemporaryEnd() const; // if exactly one neighbour is free

    bool isPit() const;
    bool isThrough() const;
    bool isExit() const; // TODO: refactor as 'hasExits'

    bool mayBeFirst() const;
    bool hasExit(int dir) const;
    const Exit* getExit(int dir) const;
    Cell* getNextCell(int dir) const;

    int getX() const;
    int getY() const;
    int getObstacleId() const;
    int getComponentId() const;
    int getMark() const;
    int getNextMask() const;
    const std::vector<Exit>& getExits() const;

    void addExit(const Exit& exit);
    void setOpposingExit(const Exit* exit, const Exit* opposingExit);

    void setType(bool obstacle);
    void setXY(int x, int y);
    void setComponentId(int id);
    void setObstacleId(int id);
    void setFree(bool free);
    void setNextMask(int mask);
    void setNextCell(int dir, Cell* ncell);
    //void setMayBeFirst(bool flag);
    void setMark(int mark);

    //void UpdateTouchingObstacles(bool inc);
};

class Exit : public Cell
{
    int dir;
    const Exit* opposingExit;

public:
    //Exit(int x, int y, int dir);
    Exit(const Cell* cell, int dir);

    const Exit* getOpposingExit() const;
    void setOpposingExit(const Exit* opposingExit);

    bool operator < (const Exit &e) const;
    bool operator == (const Exit &e) const;

    int getDir() const;
};
