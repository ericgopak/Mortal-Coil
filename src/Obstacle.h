#pragma once

#include "Common.h"
#include "AbstractComponent.h"

class Obstacle : public AbstractComponent
{
    int touchCount;

public:

    Obstacle();

    int getTouchCount() const;

    void touch();
    void untouch();
};
