#include "Common.h"
#include "Colorer.h"
#include "AbstractComponent.h"

//AbstractComponent::AbstractComponent()
//    : size(0)
//{
//}

int AbstractComponent::getSize() const
{
    return cells.size();
}

const std::set<const Cell*>& AbstractComponent::getCells() const
{
    return cells;
}

void AbstractComponent::addCell(const Cell* cell)
{
    cells.insert(cell);
}

//void AbstractComponent::incrementSize()
//{
//    size++;
//}
//
//void AbstractComponent::decrementSize()
//{
//    size--;
//}
