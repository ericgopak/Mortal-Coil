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
    Cell **Grid; // TODO: force allocate as a contiguous memory block

    int H, W;
    int solutionStartX;
    int solutionStartY;

    std::vector<Component> components;
    std::vector<Obstacle> obstacles;

    std::deque<Cell*> solution; // Sequence of Cells forming valid solution

    std::set<const Component*> specialComponents;
    std::set<int> specialComponentsIds;

    //std::set<const Cell*> temporaryEnds;
    std::vector<const Cell*> temporaryEnds;
    //std::set<const Cell*> temporaryEndsInCurrentComponent;
    std::vector<const Cell*> temporaryEndsInCurrentComponent;
    std::deque<const Component*> temporaryEndBlocks;

    void readFromFile(const char* filename);
    void init();

public:

    int componentsFullyTraversed;

    int Free; // Number of unvisited cells left
    std::vector<const Cell*> initialEnds;

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

    //const std::set<const Cell*>& getTemporaryEnds() const;
    const std::vector<const Cell*>& getTemporaryEnds() const;
    //const std::set<const Cell*>& getTemporaryEndsInCurrentComponent() const;
    const std::vector<const Cell*>& getTemporaryEndsInCurrentComponent() const;
    const std::deque<const Component*>& getTemporaryEndBlocks() const;

    int getSolutionStartX() const;
    int getSolutionStartY() const;

    const std::set<const Component*>& getSpecialComponents() const;
    const std::set<int>& getSpecialComponentIds() const;

    void setSolutionStartXY(int startX, int startY);
    void setMostCells(int index);
    void setBiggest(int index);

    void addTemporaryEnd(const Cell* cell);
    void removeTemporaryEnd(const Cell* cell);

    void addTemporaryEndsInCurrentComponent(const Cell* cell);
    void removeTemporaryEndsInCurrentComponent(const Cell* cell);

    void addTemporaryEndBlock(const Component* comp);
    void removeTemporaryEndBlock(const Component* comp);

    void addSpecialComponent(const Component* comp, int index);

    void prependSolutionCell(Cell* cell, int dir);

    void printCell(const Cell* cell, int id = -1) const;
    void traceComponent(int id = -1, unsigned int flags = TRACE_DEFAULT) const;
};
