#include "Level.h"
#include "Colorer.h"

Level::Level(const char* filename)
    : Grid(NULL)
    , H(-1)
    , W(-1)
    , componentMostCellsIndex(-1)
    , componentBiggestIndex(-1)
    , Free(0)
    , Solved(false)
    , initialEnds(0)
    , EdgesToBe(0)
    , componentsFullyTraversed(0)
{
    readFromFile(filename);
    init();

    // This should avoid calling copy constructors extra times
    components.reserve(MAX_EXPECTED_COMPONENTS);
}

Level::~Level()
{
    for (int i = 0; i < H; i++)
    {
        delete [] Grid[i];
    }
    delete [] Grid;
}

void Level::readFromFile(const char* filename)
{
    FILE* fin = fopen(filename, "r");
    assert(fin && "Failed to open file with level data!");

    fscanf(fin, "%d %d", &H, &W);

    Grid = new Cell* [H + 2];

    for (int i = 0; i <= H + 1; i++)
    {
        Grid[i] = new Cell[W + 2];

        if (1 <= i && i <= H)
        {
            for (int j = 1; j <= W; j++)
            {
                char t;
                fscanf(fin, " %c", &t);
                Grid[i][j].setType(t == 'X');
            }
        }
    }

    fclose(fin);
}

void Level::outputToFile(const char* filename) const
{
    FILE* fout = fopen(filename, "w");
    fprintf(fout, "%d %d\n", solutionStartX - 1, solutionStartY - 1); // 0-based coordinates
    fprintf(fout, "%s\n", Answer.c_str());
    fclose(fout);

    TRACE
    (
        printf("%d %d\n", solutionStartX - 1, solutionStartY - 1); // 0-based coordinates
        printf("%s\n", Answer.c_str());
    );
}

// Initialize and precalculate values
void Level::init()
{
    // Assign coordinates
    for (int i = 0; i <= H + 1; i++)
    {
        for (int j = 0; j <= W + 1; j++)
        {
            Grid[i][j].setXY(j, i);
        }
    }
    // Free cells
    for (int i = 1; i <= H; i++)
    {
        for (int j = 1; j <= W; j++)
        {
            if (Grid[i][j].isObstacle() == false)
            {
                Grid[i][j].setFree(true);
                Free++;
            }
        }
    }

    EdgesToBe = Free - 1;
}

int Level::getHeight() const
{
    return H;
}

int Level::getWidth() const
{
    return W;
}

std::vector<Component>& Level::getComponents()
{
    return components;
}

std::vector<Obstacle>& Level::getObstacles()
{
    return obstacles;
}

Cell* Level::getCell(int row, int col) const
{
    return &Grid[row][col];
}

int Level::getComponentCount() const
{
    return components.size();
}

int Level::getObstacleCount() const
{
    return obstacles.size();
}

const std::vector<const Cell*>& Level::getTemporaryEnds() const
{
    return temporaryEnds;
}

const std::vector<const Cell*>& Level::getTemporaryEndsInCurrentComponent() const
{
    return temporaryEndsInCurrentComponent;
}

const std::deque<const Component*>& Level::getTemporaryEndBlocks() const
{
    return temporaryEndBlocks;
}

int Level::getSolutionStartX() const
{
    return solutionStartX;
}

int Level::getSolutionStartY() const
{
    return solutionStartY;
}

const std::set<const Component*>& Level::getSpecialComponents() const
{
    return specialComponents;
}

const std::set<int>& Level::getSpecialComponentIds() const
{
    return specialComponentsIds;
}

void Level::setSolutionStartXY(int startX, int startY)
{
    solutionStartX = startX;
    solutionStartY = startY;
}

void Level::setMostCells(int index)
{
    componentMostCellsIndex = index;
}

void Level::setBiggest(int index)
{
    componentBiggestIndex = index;
}

void Level::addTemporaryEnd(const Cell* cell)
{
    assert(std::find(temporaryEnds.begin(), temporaryEnds.end(), cell) == temporaryEnds.end());
    temporaryEnds.push_back(cell);
}

void Level::removeTemporaryEnd(const Cell* cell)
{
    assert(cell == temporaryEnds.back());
    temporaryEnds.pop_back();
}

void Level::addTemporaryEndsInCurrentComponent(const Cell* cell)
{
    assert(std::find(temporaryEndsInCurrentComponent.begin(), temporaryEndsInCurrentComponent.end(), cell) == temporaryEndsInCurrentComponent.end());
    temporaryEndsInCurrentComponent.push_back(cell);
}

void Level::removeTemporaryEndsInCurrentComponent(const Cell* cell)
{
    assert(temporaryEndsInCurrentComponent.back() == cell);
    temporaryEndsInCurrentComponent.pop_back();
}

void Level::addTemporaryEndBlock(const Component* comp)
{
    temporaryEndBlocks.push_back(comp);
}

void Level::removeTemporaryEndBlock(const Component* comp)
{
    assert(temporaryEndBlocks.empty() == false);
    if (comp == temporaryEndBlocks.back())
    {
        temporaryEndBlocks.pop_back();
    }
    else if (comp == temporaryEndBlocks.front())
    {
        temporaryEndBlocks.pop_front();
    }
    else
    {
        assert(false && "Meh, temporary end blocks appeared to be mixed up!");
    }
}

void Level::addSpecialComponent(const Component* comp, int index)
{
    specialComponents.insert(comp);
    specialComponentsIds.insert(index);
}

void Level::prependSolutionCell(Cell* cell, int dir)
{
    solution.push_front(cell);
}

void Level::printCell(const Cell* cell, int id) const
{
    ColorType background = 0x00;
    ColorType foreground = 0x00;
    char symbol = '?';

    // Background
    if (cell->isObstacle())
    {
        background = BACKGROUND_GREEN;
        if (cell->isFree()) // Special case: temporary obstacle
        {
            background = BACKGROUND_GRAY;
        }
    }
    else if (components[cell->getComponentId()].isPortal())
    {
        background = BACKGROUND_RED;
    }
    else
#ifdef TRACE_SOLUTIONS
        if (cell->getLayer() != -1)
    {
        background = BACKGROUND_INTENSITY | BACKGROUND_ONE * (2 + cell->getLayer());
    }
    else
#endif
        if (std::find(temporaryEnds.begin(), temporaryEnds.end(), cell) != temporaryEnds.end())
    {
        background = BACKGROUND_YELLOW;
    }
    else if (cell->isFree() == false)
    {
        background = BACKGROUND_GRAY;
    }
    else
    {
        background = BACKGROUND_WHITE;
    }

    // Foreground
#ifdef TRACE_SOLUTIONS
    if (cell->getDepth() != -1)
    {
        foreground = GRAY;

        if (cell->getX() == Debug::currentX && cell->getY() == Debug::currentY)
        {
            foreground = YELLOW;
        }
    }
    else
#endif
        if (id != -1 && cell->getComponentId() == id)
    {
        foreground = RED;
    }

    // Symbol to print
    if (cell->isObstacle())
    {
        symbol = '0' + cell->getObstacleId() % 10;
    }
    else
#ifdef TRACE_SOLUTIONS
        if (cell->getDepth() != -1)
    {
        symbol = '0' + (cell->getDepth() + 1) % 10;
        TRACE(
            if (cell->getX() == Debug::currentX && cell->getY() == Debug::currentY)
            {
                symbol = '@';
            }
        );
    }
    else
#endif
        if (id != -1 && cell->getComponentId() == id)
    {
        symbol = '*';
    }
    else if (cell->getComponentId() != -1)
    {
        symbol = '0' + cell->getComponentId() % 10;
    }

    Colorer::setColor(foreground | background);
    printf("%c", symbol);
    Colorer::restoreColor();
}

void Level::traceComponent(int id, unsigned int flags) const
{
    if (id != -1)
    {
        printf("Tracing %s %d\n", (flags & TRACE_COMPONENTS) ? "component": "obstacle", id);
        //printf("Size: %d\n", (flags & TRACE_COMPONENTS) ? components[id].getSize() : obstacles[id].getSize());
    }

    int x1 = 1, x2 = W;
    int y1 = 1, y2 = H;

    if (id != -1)
    {
        x1 = W, x2 = 1;
        y1 = H, y2 = 1;
        for (int i = 1; i <= H; i++)
        {
            for (int j = 1; j <= W; j++)
            {
                if (Grid[i][j].getComponentId() == id)
                {
                    if (j < x1) x1 = j;
                    if (j > x2) x2 = j;
                    if (i < y1) y1 = i;
                    if (i > y2) y2 = i;
                }
            }
        }
    }

    x1--;
    x2++;
    y1--;
    y2++;

    for (int i = y1; i <= y2; i++)
    {
        for (int j = x1; j <= x2; j++)
        {
            if ((flags & TRACE_COMPONENTS) && Grid[i][j].isObstacle() == false)
            {
                printCell(&Grid[i][j], id);
            }
            else if ((flags & TRACE_OBSTACLES) && Grid[i][j].isObstacle())
            {
                printCell(&Grid[i][j], id);
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}
