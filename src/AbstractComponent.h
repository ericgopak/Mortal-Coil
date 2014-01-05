#pragma once

#include "Common.h"

/* Ideas:
o store set of Cells's for convenience
o replace (in,de)crementSize() with (de,in)crementOccupied
*/

class AbstractComponent
{
protected:
    std::set<const Cell*> cells;

public:
    //AbstractComponent();

    int getSize() const;
    const std::set<const Cell*>& getCells() const;

    void addCell(const Cell* cell);
};
