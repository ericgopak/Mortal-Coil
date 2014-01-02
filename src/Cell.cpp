#include "Common.h"
#include "Cell.h"

Cell::Cell(int x, int y)
    : x(x)
    , y(y)
    , free(false)
    , type(Obstacle)
    , obstacleId(-1)
    , componentId(-1)
    , nextMask(0)
    , touch(0)
    , mark(0)
    , exitMask(0)
{
    nextCell[0] = nextCell[1] = nextCell[2] = nextCell[3] = NULL;
    exitsByDirection[0] = exitsByDirection[1] = exitsByDirection[2] = exitsByDirection[3] = NULL;
    for (int i = 0; i < MAX_CAN_TOUCH; i++)
    {
        nexttouch[i] = 0;
        for (int j = 0; j < MAX_NEXT_TOUCH; j++)
        {
            NextTouch[i][j] = NULL;
        }
    }
}

Cell::~Cell()
{
}

bool Cell::isFree() const
{
    return free;
}

bool Cell::isObstacle() const
{
    return type == Cell::Obstacle;
}

bool Cell::isOccupied() const
{
    return free == false;
}

bool Cell::isEnd() const
{
    if (!free)
    {
        return false;
    }

    int c = 0;
    for (int dir = 0; dir < 4; dir++)
    {
        c += nextCell[dir]->isFree();
    }
    return c == 1;
}

bool Cell::isPit() const
{
    return Bits[nextMask] <= 1;
}

bool Cell::isThrough() const
{
    int k = 0;
    
    for (int dir = 0; dir < 4; dir++)
    {
        if ((nextMask & P[dir]) && Bits[nextCell[dir]->getNextMask()] == 2)
        {
            k++;
        }
    }

    return k == 2;
}

bool Cell::isExit() const
{
    return getExits().size() > 0;
}

bool Cell::mayBeFirst() const
{
    return getExits().size() > 0;
}

bool Cell::hasExit(int dir) const
{
    return (exitMask & (1 << dir)) != 0;
}

const Exit* Cell::getExit(int dir) const
{
    return exitsByDirection[dir];
}

Cell* Cell::getNextCell(int dir) const
{
    return nextCell[dir];
}

int Cell::getX() const
{
    return x;
}

int Cell::getY() const
{
    return y;
}

int Cell::getObstacleId() const
{
    return obstacleId;
}

int Cell::getComponentId() const
{
    return componentId;
}

int Cell::getMark() const
{
    return mark;
}

int Cell::getNextMask() const
{
    return nextMask;
}

const std::set<Exit>& Cell::getExits() const
{
    return exits;
}

void Cell::addExit(const Exit& exit)
{
    exits.insert(exit);
    exitMask |= 1 << exit.getDir();
    exitsByDirection[exit.getDir()] = &*(exits.find(exit)); // TODO: this one seems to be slow
}

void Cell::setType(bool obstacle)
{
    type = obstacle ? Cell::Obstacle : Cell::FreeCell;
}

void Cell::setXY(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Cell::setComponentId(int id)
{
    componentId = id;
}

void Cell::setObstacleId(int id)
{
    obstacleId = id;
}

void Cell::setFree(bool free)
{
    this->free = free;
}

void Cell::setNextMask(int mask)
{
    nextMask = mask;
}

void Cell::setNextCell(int dir, Cell* ncell)
{
    nextCell[dir] = ncell;
}

void Cell::setMark(int mark)
{
    this->mark = mark;
}

//Exit::Exit(int x, int y, int dir)
//    : Cell(x, y)
//    , dir(dir)
//{
//}

Exit::Exit(const Cell* cell, int dir)
    : Cell(*cell)
    , dir(dir)
    , opposite(NULL)
{
}

//const Exit* Exit::getOpposite() const
//{
//    return opposite;
//}
//
//void Exit::setOpposite(const Exit* exit)
//{
//    opposite = exit;
//}

bool Exit::operator < (const Exit &e) const
{
    if (y != e.y)
    {
        return y < e.y;
    }
    if (x != e.x)
    {
        return x < e.x;
    }
    return dir < e.dir;
}

bool Exit::operator == (const Exit &e) const
{
    return x == e.x && y == e.y && dir == e.dir;
}

int Exit::getDir() const
{
    return dir;
}
