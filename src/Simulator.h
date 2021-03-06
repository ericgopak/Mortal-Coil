#pragma once

#include <vector>
#include "Level.h"
#include "Obstacle.h"

class Level;
class Cell;

class Simulator
{
    TRACE(
        struct TraceInfo
        {
            int depth;
            int layer;

            TraceInfo();
        };
    )

    Simulator& operator = (const Simulator&);
    Simulator(const Simulator&);

protected:
    Level* level;

public:
    TRACE(
        TraceInfo tracer;
    );

    Simulator(Level* currentLevel);
    ~Simulator();

    void occupy(Cell* cell, int dir);
    void restore(Cell* cell, int dir);
    Cell* moveForward(Cell* cell, int dir);
    Cell* moveBackwards(Cell* cell, int dir);

    void findComponents() const;
    void findObstacles() const;
    void floodComponent(int x, int y, int num) const;
    void floodObstacle(int x, int y, int num) const;
    void floodInnerSubcomponents(Component* comp);
    void floodInnerSubcomponents();
    void findComponentExits() const;

    void findTouchingObstacles() const;

    void findOpposingExits() const;

    void findNeighbours() const;
    void createBonds() const;
    void countEnds() const;
    
    void touchObstacles(Cell* cell) const;
    bool checkTouchingObstacles(Cell* cell) const;
    void untouchObstacles(Cell* cell) const;

    //void backtrack(Cell* cell, int direction);

    //virtual void trySolving(int startX, int startY);

    virtual bool mayStartFrom(Cell* cell, int dir) const;
    virtual bool shouldConsider(Cell* cell, int dir) const;
    virtual bool stopBacktracking() const;

    virtual void preOccupyAction(Cell* cell, int dir) = 0;
    virtual void postOccupyAction(Cell* cell, int dir) = 0;
    virtual void preRestoreAction(Cell* cell, int dir) = 0;
    virtual void postRestoreAction(Cell* cell, int dir) = 0;
    virtual void preAction(Cell* cell, int dir) = 0;
    virtual void postAction(Cell* cell, int dir) = 0;
    
    virtual bool potentialSolution(Cell* cell, int dir) const = 0;
    virtual void solutionFound(Cell* cell, int dir) = 0;
    virtual bool reachedFinalCell(Cell* cell, int dir) const = 0;
};
