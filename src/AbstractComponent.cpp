#include "Common.h"
#include "Colorer.h"
#include "AbstractComponent.h"

AbstractComponent::AbstractComponent()
    : size(0)
{
}

int AbstractComponent::getSize() const
{
    return size;
}

void AbstractComponent::incrementSize()
{
    size++;
}

void AbstractComponent::decrementSize()
{
    size--;
}
