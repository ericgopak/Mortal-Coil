#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Cell.h"

Obstacle::Obstacle()
    : AbstractComponent()
    , touchCount(0)
{
}

//int Obstacle::getTouchCount() const
//{
//    return touchCount;
//}

int Obstacle::getTouchCount() const
{
    return (int)touches.size();
}

const Cell* Obstacle::getFirstTouch() const
{
    return touches.front();
}

const Cell* Obstacle::getLastTouch() const
{
    return touches.back();
}

static bool cellsAreNeighbours(const Cell* c1, const Cell* c2)
{
    int dx = c1->getX() - c2->getX();
    int dy = c1->getY() - c2->getY();
    return dx == 0 && abs(dy) == 1
        || dy == 0 && abs(dx) == 1;
}

bool Obstacle::touch(const Cell* cell)
{
//Colorer::print<GREEN>("touching... (%d,%d)\n", cell->getX(), cell->getY());
    if (getTouchCount() == 0)
    {
        touches.push_back(cell);
        return true;
    }

    const auto& c1 = getFirstTouch();
    const auto& c2 = getLastTouch();
    if (cellsAreNeighbours(c1, cell))
    {
        touches.push_front(cell);
        return true;
    }
    else if (cellsAreNeighbours(c2, cell))
    {
        touches.push_back(cell);
        return true;
    }

    return false;
}

void Obstacle::untouch(const Cell* cell)
{
//Colorer::print<GREEN>("untouching... (%d,%d)\n", cell->getX(), cell->getY());
    const auto& c1 = getFirstTouch();
    const auto& c2 = getLastTouch();
    if (c1 == cell)
    {
        touches.pop_front();
    }
    else if (c2 == cell)
    {
        touches.pop_back();
    }
    else
    {
        assert(false && "Should never happen!");
    }
}
