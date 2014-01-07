#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

class Cell
{
    enum CellType
    {
        FreeCell,
        Obstacle
    };

protected:
    CellType type;
    int x, y;
    bool free;

public:

    Cell(int x = -1, int y = -1);

    bool isObstacle() const;
    bool isFree() const;
    bool isOccupied() const;

    int getX() const;
    int getY() const;

    void setType(bool obstacle);
    void setXY(int x, int y);
    void setFree(bool free);
};

class Level
{
    Cell **Grid;

    int H, W;
    int solutionStartX;
    int solutionStartY;

    void readFromFile(const char* filename);
    void init();

public:

    int Free; // Number of unvisited cells left
    bool Solved; // True if any solution has been found
    std::string Answer; // Solution (reversed)

    Level(const char* filename);
    ~Level();

    int getHeight() const;
    int getWidth() const;

    Cell* getCell(int row, int col) const;

    int getSolutionStartX() const;
    int getSolutionStartY() const;

    void setSolutionStartXY(int startX, int startY);
    
    void occupy(Cell* cell);
};

Level::Level(const char* filename)
    : Grid(NULL)
    , H(-1)
    , W(-1)
    , Free(0)
    , Solved(false)
{
    readFromFile(filename);
    init();
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

    if (!fin)
    {
        perror("Failed to open level file!");
        exit(EXIT_FAILURE);
    }

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

//    for (int i = 0; i <= H + 1; i++)
//    {
//        for (int j = 0; j <= W + 1; j++)
//        {
//            printf("%d", !Grid[i][j].isObstacle());
//        }
//        printf("\n");
//    }

    fclose(fin);
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
}

int Level::getHeight() const
{
    return H;
}

int Level::getWidth() const
{
    return W;
}

Cell* Level::getCell(int row, int col) const
{
    return &Grid[row][col];
}

void Level::setSolutionStartXY(int startX, int startY)
{
    solutionStartX = startX;
    solutionStartY = startY;
}

int Level::getSolutionStartX() const
{
    return solutionStartX;
}

int Level::getSolutionStartY() const
{
    return solutionStartY;
}

Cell::Cell(int _x, int _y)
    : x(_x)
    , y(_y)
    , free(false)
    , type(Obstacle)
{
}

bool Cell::isFree() const
{
    return free;
}

bool Cell::isObstacle() const
{
    return type == Cell::Obstacle;
}

bool Cell::isOccupied() const
{
    return free == false;
}

int Cell::getX() const
{
    return x;
}

int Cell::getY() const
{
    return y;
}

void Cell::setType(bool obstacle)
{
    type = obstacle ? Cell::Obstacle : Cell::FreeCell;
}

void Cell::setXY(int _x, int _y)
{
    x = _x;
    y = _y;
}

void Cell::setFree(bool _free)
{
    free = _free;
}

void readSolution(Level* level)
{
    int sx, sy;
    std::cin >> sx >> sy; // 0-based
    
    level->setSolutionStartXY(sx + 1, sy + 1);
    
    std::string s;
    while (std::cin >> s)
    {
        level->Answer += s;
    }
}

void Level::occupy(Cell* cell)
{
    if (cell->isFree() == false)
    {
        perror("Occupying already non-free cell!");
    }
    cell->setFree(false);
    Free--;
}

const int dx[8] = {1, 0,-1, 0,  1,-1,-1, 1};
const int dy[8] = {0, 1, 0,-1,  1, 1,-1,-1};
//const int Left[4]  = {3, 0, 1, 2};
//const int Right[4] = {1, 2, 3, 0};
//const char Direction[4] = {'R', 'D', 'L', 'U'};

bool testSolution(Level* level)
{
    const std::string& answer = level->Answer;
    
    //printf("Starting simulation from (%d,%d)\n", level->getSolutionStartX(), level->getSolutionStartY());
    
    int currDir = -1;
    int currX = level->getSolutionStartX();
    int currY = level->getSolutionStartY();
    Cell* currCell = level->getCell(currY, currX);
    level->occupy(currCell);
    
    for (size_t i = 0; i < answer.size(); i++)
    {
        switch(answer[i])
        {
        case 'R':
            currDir = 0;
            break;
        case 'D':
            currDir = 1;
            break;
        case 'L':
            currDir = 2;
            break;
        case 'U':
            currDir = 3;
            break;
        default:
            perror("Invalid answer format!");
            return false;
        }
        
        Cell* nextCell = level->getCell(currY + dy[currDir], currX + dx[currDir]);
        
        if (nextCell->isFree())
        {
            while (nextCell->isFree())
            {
                level->occupy(nextCell);
                
                currCell = nextCell;
                currX += dx[currDir];
                currY += dy[currDir];
                //printf("Moving to (%d, %d)...\n", currX, currY);
                nextCell = level->getCell(currY + dy[currDir], currX + dx[currDir]);
            }
        }
        else
        {
            printf("Sorry, the solution is wrong: fail at x = %d, y = %d, answer[%d] = %c\n", currX, currY, i, answer[i]);
            return false;
        }
    }
    
    if (level->Free == 0)
    {
        return true;
    }
    
    return false;
}

int main()
{
    Level level("mortal_coil.txt");
    
    readSolution(&level);
    
    if (level.Answer.size() == 0)
    {
        perror("Answer is empty");
        return -1;
    }
    
    if (testSolution(&level))
    {
        printf("OK");
    }
    else
    {
        printf("FAIL");
    }
    
    return 0;
}
