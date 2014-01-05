#include "Simulator.h"
#include "Colorer.h"
#include "Common.h"
#include "Component.h"

#include <unordered_set>

static bool isExit(const Cell* cell, int dir)
{
    const Cell* left = cell->getNextCell(Left[dir]);
    const Cell* frontLeft = left->getNextCell(dir);
    const Cell* right = cell->getNextCell(Right[dir]);
    const Cell* frontRight = right->getNextCell(dir);

    return (left->isObstacle() || frontLeft->isObstacle()) && (right->isObstacle() || frontRight->isObstacle());
}

Simulator::Simulator(Level* currentLevel)
    : level(currentLevel)
{
}

Simulator::~Simulator()
{
}

Simulator::TraceInfo::TraceInfo()
    : depth(0)
    , currentComponent(-1)
    , currentX(-1)
    , currentY(-1)
{
}

void Simulator::occupy(Cell* cell, int dir) const
{
    assert(cell->isFree() && "Occupying already non-free cell!");

    cell->setFree(false);
    level->Free--;
    level->getComponents()[cell->getComponentId()].incrementOccupied();
}

void Simulator::restore(Cell* cell, int dir) const
{
    assert(cell->isFree() == false && "Restoring already free cell!");

    cell->setFree(true);
    level->Free++;
    level->getComponents()[cell->getComponentId()].decrementOccupied();
}

Cell* Simulator::moveForward(Cell* cell, int dir) const
{
    preOccupyAction(cell, dir);
    occupy(cell, dir);
    postOccupyAction(cell, dir);

    return cell->getNextCell(dir);
}

Cell* Simulator::moveBackwards(Cell* cell, int dir) const
{
    cell = cell->getNextCell(dir ^ 2);

    preRestoreAction(cell, dir);
    restore(cell, dir);
    postRestoreAction(cell, dir);

    return cell;
}

void Simulator::findComponents() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false && level->getCell(i, j)->getComponentId() == -1)
            {
                int index = level->getComponentCount();
                
                level->getComponents().push_back(Component());

                floodComponent(j, i, index);
            }
        }
    }
}

void Simulator::findTouchingObstacles() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            Cell* c1 = level->getCell(i, j);
            if (c1->isObstacle() == false)
            {
                for (int dir = 0; dir < 8; dir++)
                {
                    Cell* c2 = level->getCell(i + dy[dir], j + dx[dir]);
                    if (c2->isObstacle())
                    {
                        const Cell* leftCell  = c1->getNextCell(Left[dir]);
                        const Cell* rightCell = c1->getNextCell(Right[dir]);

                        if (leftCell->isObstacle() == false)
                        {
                            c1->addNextTouch(c2->getObstacleId(), leftCell);
                        }
                        if (rightCell->isObstacle() == false)
                        {
                            c1->addNextTouch(c2->getObstacleId(), rightCell);
                        }
                    }
                }
            }
        }
    }
}

void Simulator::findObstacles() const
{
    // Special obstacle - the one that includes borders
    level->getObstacles().push_back(Obstacle());
    floodObstacle(0, 0, 0);

    // Remaining obstacles
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() && level->getCell(i, j)->getObstacleId() == -1)
            {
                int index = level->getObstacleCount();
                level->getObstacles().push_back(Obstacle());
                // New obstacle found -> mark all the cells in it
                floodObstacle(j, i, index);
            }
        }
    }
}

void Simulator::floodObstacle(int x, int y, int num) const
{
    Cell* cell = level->getCell(y, x);
    cell->setObstacleId(num);
    level->getObstacles()[num].addCell(cell);

    for (int dir = 0; dir < 8; dir++) // Diagonal directions as well
    {
        int ny = y + dy[dir];
        int nx = x + dx[dir];

        if (0 <= ny && ny <= level->getHeight() + 1)
        if (0 <= nx && nx <= level->getWidth() + 1)
        if (level->getCell(ny, nx)->isObstacle())
        if (level->getCell(ny, nx)->getObstacleId() == -1) // Not assigned
        {
            floodObstacle(nx, ny, num);
        }
    }
}

static void collectExitsAlongPerimeter(Cell* cell, int dir, const Cell* const entryPoint, Component* const component)
{
    Cell* left = cell->getNextCell(Left[dir]);
    if (!left->isObstacle())
    {
        if (left->getComponentId() == cell->getComponentId())
        {
            if (left != entryPoint)
            {
                collectExitsAlongPerimeter(left, Left[dir], entryPoint, component);
            }
            return;
        }
        else // is exit
        {
            Exit exit(cell, Left[dir]);
            cell->addExit(exit);
            component->addExit(cell->getExit(Left[dir]));
        }
    }

    Cell* front = cell->getNextCell(dir);
    if (!front->isObstacle())
    {
        if (front->getComponentId() == cell->getComponentId())
        {
            if (front != entryPoint)
            {
                collectExitsAlongPerimeter(front, dir, entryPoint, component);
            }

            if (cell->hasExits())
            {
                component->addExitCell(cell);
            }
            return;
        }
        else // is exit
        {
            Exit exit(cell, dir);
            cell->addExit(exit);
            component->addExit(cell->getExit(dir));
        }
    }

    Cell* right = cell->getNextCell(Right[dir]);
    if (!right->isObstacle())
    {
        if (right->getComponentId() == cell->getComponentId())
        {
            if (right != entryPoint)
            {
                collectExitsAlongPerimeter(right, Right[dir], entryPoint, component);
            }

            if (cell->hasExits())
            {
                component->addExitCell(cell);
            }
            return;
        }
        else // is exit
        {
            Exit exit(cell, Right[dir]);
            cell->addExit(exit);
            component->addExit(cell->getExit(Right[dir]));
        }
    }

    if (cell->hasExits())
    {
        component->addExitCell(cell);
    }
}

// TODO: this does not consider exits to inner components (between inner obstacles)
void Simulator::findComponentExits() const
{
    std::unordered_set<int> processedComponents;

    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            Cell* cell = level->getCell(i, j);
            int componentId = cell->getComponentId();
            if (!cell->isObstacle() && processedComponents.find(componentId) == processedComponents.end())
            {
                Component* comp = &level->getComponents()[componentId];

                // Assumption: first cell encountered is along the perimeter
                collectExitsAlongPerimeter(cell, 3, cell, comp); // Start by going up. We know there is an obstacle to the left

                if (comp->getSize() > 1 && comp->getExitCells().size() == 1)
                {
                    level->addTemporaryEndBlock(comp);
                }

                processedComponents.insert(componentId);
            }
        }
    }
}

void Simulator::findOpposingExits() const
{
    FOREACH(level->getComponents(), comp)
    {
        FOREACH(comp->getExits(), e)
        {
            const Exit* exit = *e;
            Cell* cell = level->getCell(exit->getY(), exit->getX());
            int dir = exit->getDir();
            cell->setOpposingExit(exit, exit->getNextCell(dir)->getExit(dir ^ 2));
        }
    }
}

void Simulator::findNeighbours() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false)
            {
                for (int dir = 0; dir < 4; dir++)
                {
                    if (level->getCell(i + dy[dir], j + dx[dir])->isObstacle() == false)
                    {
                        level->getCell(i, j)->setNextMask(level->getCell(i, j)->getNextMask() | P[dir]);
                    }
                }
            }
        }
    }
}

void Simulator::createBonds() const
{
    for (int i = 0; i <= level->getHeight() + 1; i++)
    {
        for (int j = 0; j <= level->getWidth() + 1; j++)
        {
            for (int dir = 0; dir < 8; dir++)
            {
                if (0 <= i + dy[dir] && i + dy[dir] <= level->getHeight() + 1)
                if (0 <= j + dx[dir] && j + dx[dir] <= level->getWidth() + 1)
                {
                    level->getCell(i, j)->setNextCell(dir, level->getCell(i + dy[dir], j + dx[dir]));
                }
            }
        }
    }
}

void Simulator::countEnds() const
{
    for (int i = 1; i <= level->getHeight(); i++)
    {
        for (int j = 1; j <= level->getWidth(); j++)
        {
            if (level->getCell(i, j)->isObstacle() == false && level->getCell(i, j)->isPit())
            {
                level->initialEnds.push_back(level->getCell(i, j));
            }
        }
    }
}

void Simulator::floodComponent(int x, int y, int num) const
{
    Cell* cell = level->getCell(y, x);

    cell->setComponentId(num);
    level->getComponents()[num].addCell(cell);

    for (int dir = 0; dir < 4; dir++)
    {
        int ny = y + dy[dir];
        int nx = x + dx[dir];

        if (1 <= ny && ny <= level->getHeight())
        {
            if (1 <= nx && nx <= level->getWidth())
            {
                Cell* ncell = level->getCell(ny, nx);

                if (ncell->isObstacle() == false)
                {
                    if (isExit(cell, dir) == false)
                    {
                        if (ncell->getComponentId() == -1)
                        {
                            floodComponent(nx, ny, num);
                        }
                    }
                }
            }
        }
    }
}

void Simulator::touchObstacles(Cell* cell) const
{
    FOREACH_CONST(cell->getNeighboursTouchingSameObstacle(), it)
    {
        level->getObstacles()[it->first].touch();
    }
}

bool Simulator::checkTouchingObstacles(Cell* cell) const
{
    int x = cell->getX();
    int y = cell->getY();

    FOREACH_CONST(cell->getNeighboursTouchingSameObstacle(), it)
    {
        const Obstacle& o = level->getObstacles()[it->first];

        if (o.getTouchCount() > 0)
        {
            bool ok = false;

            FOREACH_CONST(it->second, it2)
            {
                if ((*it2)->isFree() == false)
                {
                    ok = true;
                    break;
                }
            }
            if (ok == false)
            {
                return false;
            }
        }
    }

    return true;
}

void Simulator::untouchObstacles(Cell* cell) const
{
    FOREACH_CONST(cell->getNeighboursTouchingSameObstacle(), it)
    {
        level->getObstacles()[it->first].untouch();
    }
}

bool Simulator::mayStartFrom(Cell* cell, int dir) const
{
    if (cell->getNextCell(dir)->isFree() == false)
    {
        return false;
    }
    if (cell->isThrough())
    {
        return false;
    }
    return true;
}

bool Simulator::shouldConsider(Cell* cell, int dir) const
{
    if (!cell->isFree())
    {
        return false;
    }
    return true;
}

bool Simulator::stopBacktracking() const
{
    return level->Solved;
}

void Simulator::trySolving(int startX, int startY)
{
    Cell* cell = level->getCell(startY, startX);

    for (int d = 0; d < 4; d++)
    {
        if (mayStartFrom(cell, d))
        {
            backtrack(cell, d);

            if (stopBacktracking())
            {
                return;
            }
        }
    }
}

void Simulator::backtrack(Cell* cell, int direction)
{
    if (!shouldConsider(cell, direction))
    {
        return;
    }

    //level->traceComponent();
    TRACE(tracer.depth++);

    preAction(cell, direction);

    if (potentialSolution(cell, direction))
    {
        if (reachedFinalCell(cell, direction))
        {
            solutionFound(cell, direction);
        }
        else
        {
            if (cell->getNextCell(direction)->isFree()) // Forward
            {
                cell = moveForward(cell, direction);
                backtrack(cell, direction);
                cell = moveBackwards(cell, direction);
            }
            else
            {
                // Turn left
                int leftDirection = Left[direction];
                if (cell->getNextCell(leftDirection)->isFree())
                {
                    cell = moveForward(cell, leftDirection);
                    backtrack(cell, leftDirection);
                    cell = moveBackwards(cell, leftDirection);
                }

                if (stopBacktracking() == false) // Do not terminate in order to guarantee proper postAction()
                {
                    // Turn right
                    int rightDirection = Right[direction];
                    if (cell->getNextCell(rightDirection)->isFree())
                    {
                        cell = moveForward(cell, rightDirection);
                        backtrack(cell, rightDirection);
                        cell = moveBackwards(cell, rightDirection);
                    }
                }
            }
        }
    }

    postAction(cell, direction);

    TRACE(tracer.depth--);
}
