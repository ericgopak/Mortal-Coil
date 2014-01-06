#include "Common.h"
#include "Colorer.h"
#include "AbstractComponent.h"

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
