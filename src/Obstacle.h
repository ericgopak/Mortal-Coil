#pragma once

#include "Common.h"
#include "AbstractComponent.h"

class Obstacle : public AbstractComponent
{
    int touched; // How many touching free cells have been visited

public:
    //static int Count;

    Obstacle();

    // Print out current grid state
    //static void Trace();

    int Touched() const;

    void IncrementTouched();
    void DecrementTouched();
};
