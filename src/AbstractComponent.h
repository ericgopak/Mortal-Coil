#pragma once

#include "Common.h"

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
