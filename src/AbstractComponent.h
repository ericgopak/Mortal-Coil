#pragma once

#include "Common.h"

class AbstractComponent
{
protected:
    std::set<const Cell*> cells;

public:
    int getSize() const;
    const std::set<const Cell*>& getCells() const;

    void addCell(const Cell* cell);
};
