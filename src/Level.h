#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <deque>
#include "Common.h"
#include "Cell.h"
#include "Component.h"
#include "Obstacle.h"

static const unsigned int TRACE_COMPONENTS = 0x01;
static const unsigned int TRACE_OBSTACLES  = 0x02;
static const unsigned int TRACE_DEFAULT = TRACE_COMPONENTS | TRACE_OBSTACLES;

class Level
{
    Cell **Grid;

    int H, W;
    int solutionStartX;
    int solutionStartY;

    void readFromFile(const char* filename);
    void init();

    std::vector<Component> components;
    std::vector<Obstacle> obstacles;

    std::deque<Cell*> solution; // Sequence of Cells forming valid solution

    std::set<const Component*> specialComponents;

public:

    int componentsFullyTraversed;

    int Free; // Number of unvisited cells left
    int Ends; // Number of pits currently on the board
    int EndX[2]; // Coordinates of those ends
    int EndY[2];

    bool Solved; // True if any solution has been found
    bool Analyzed; // True if any solution to currently being considered component has been found
    std::string Answer;

    int EdgesToBe; // Number of edges required for the resulting spanning tree

    // Statistics
    int componentMostCellsIndex;
    int componentBiggestIndex;

    Level(const char* filename);
    ~Level();

    void outputToFile(const char* filename) const;

    int getHeight() const;
    int getWidth() const;

    std::vector<Component>& getComponents();
    std::vector<Obstacle>& getObstacles();

    Cell* getCell(int row, int col) const;

    int getComponentCount() const;
    int getObstacleCount() const;

    int getSolutionStartX() const;
    int getSolutionStartY() const;

    const std::set<const Component*>& getSpecialComponents() const;

    void setSolutionStartXY(int startX, int startY);
    void setMostCells(int index);
    void setBiggest(int index);

    void addSpecialComponent(const Component* comp);

    void prependSolutionCell(Cell* cell, int dir);
    void prepareSolution();

    void PrintCell(const Cell* cell, int id = -1) const;
    void traceComponent(int id = -1, unsigned int flags = TRACE_DEFAULT) const;
};
