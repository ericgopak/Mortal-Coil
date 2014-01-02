#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"

//int Obstacle::Count = 0;

Obstacle::Obstacle()
    : AbstractComponent()
    , touched(0)
{
}

//void Obstacle::Trace()
//{
//    AbstractComponent::Trace(-1, AbstractComponent::TRACE_OBSTACLES);
//}

int Obstacle::Touched() const
{
    return touched;
}

void Obstacle::IncrementTouched()
{
    touched++;
}

void Obstacle::DecrementTouched()
{
    touched--;
}
