#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"

Obstacle::Obstacle()
    : AbstractComponent()
    , touchCount(0)
{
}

int Obstacle::getTouchCount() const
{
    return touchCount;
}

void Obstacle::touch()
{
    touchCount++;
}

void Obstacle::untouch()
{
    touchCount--;
}
