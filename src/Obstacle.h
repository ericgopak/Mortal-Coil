#pragma once

#include "Common.h"
#include "AbstractComponent.h"

#include <deque>

class Obstacle : public AbstractComponent
{
    int touchCount;
    std::deque<const Cell*> touches;

public:

    Obstacle();

    int getTouchCount() const;
    const Cell* getFirstTouch() const;
    const Cell* getLastTouch() const;

    bool touch(const Cell* cell);
    void untouch(const Cell* cell);
};
