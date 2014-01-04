#pragma once

#include "Common.h"

/* Ideas:
o store set of Cells's for convenience
o replace (in,de)crementSize() with (de,in)crementOccupied
*/

class AbstractComponent
{
protected:
    int size;

public:
    AbstractComponent();

    int getSize() const;

    void incrementSize();
    void decrementSize();
};
