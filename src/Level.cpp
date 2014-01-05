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

    /*for (int i = 0; i <= H + 1; i++)
    {
        for (int j = 0; j <= W + 1; j++)
        {
            printf("%d", !Grid[i][j].isObstacle());
        }
        printf("\n");
    }*/

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

const std::set<const Cell*>& Level::getTemporaryEnds() const
{
    return temporaryEnds;
}

const std::set<const Component*>& Level::getTemporaryEndBlocks() const
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
    temporaryEnds.insert(cell);
}

void Level::removeTemporaryEnd(const Cell* cell)
{
    temporaryEnds.erase(cell);
}

void Level::addTemporaryEndBlock(const Component* comp)
{
    temporaryEndBlocks.insert(comp);
}

void Level::removeTemporaryEndBlock(const Component* comp)
{
    temporaryEndBlocks.erase(comp);
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

void Level::prepareSolution(int startX, int startY)
{
    setSolutionStartXY(startX, startY);

    Answer.clear();

    int currDir = -1;
    for (size_t i = 1; i < solution.size(); i++)
    {
        const Cell* a = solution[i - 1];
        const Cell* b = solution[i];
        int x1 = a->getX();
        int y1 = a->getY();
        int x2 = b->getX();
        int y2 = b->getY();
        
        int dir = -1;
        if (x1 < x2)
        {
            dir = 0;
        }
        else if (y1 < y2)
        {
            dir = 1;
        }
        else if (x1 > x2)
        {
            dir = 2;
        }
        else if (y1 > y2)
        {
            dir = 3;
        }
        else
        {
            assert(false && "Something's wrong with the answer - found cells with equal coordinates!");
        }
        
        if (dir != currDir)
        {
            Answer += Direction[dir];
            currDir = dir;
        }
    }
}

void Level::PrintCell(const Cell* cell, int id) const
{
    if (cell->isObstacle())
    {
        Colorer::print<BACKGROUND_GREEN>("%d", cell->getObstacleId() % 10);
    }
    else if (cell->getMark())
    {
        Colorer::setColor((3 + cell->getMark() % 4) | BACKGROUND_WHITE);
        printf("%d", cell->getMark() % 10);
        Colorer::restoreColor();
    }
    else if (cell->mayBeFirst())
    {
        Colorer::print<BACKGROUND_WHITE | BACKGROUND_INTENSITY>("%d", cell->getComponentId() % 10);
    }
    else if (id != -1 && cell->getComponentId() == id)
    {
        Colorer::print<RED | BACKGROUND_WHITE>("*");
        Colorer::restoreColor();
    }
    else if (cell->getComponentId() != -1)
    {
        Colorer::setColor(BACKGROUND_WHITE);
        printf("%d", cell->getComponentId() % 10);
        Colorer::restoreColor();
    }
    else
    {
        printf("?");
    }
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
                PrintCell(&Grid[i][j], id);
            }
            else if ((flags & TRACE_OBSTACLES) && Grid[i][j].isObstacle())
            {
                PrintCell(&Grid[i][j], id);
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}
