#include "Common.h"
#include "Colorer.h"
#include "Obstacle.h"
#include "Component.h"
#include "Analyzer.h"
#include "Solver.h"

static const char* INPUT_FILENAME = "mortal_coil.txt";
static const char* OUTPUT_FILENAME = "output.txt";

void printUsage()
{
    Colorer::print<YELLOW>(
        "Usage:\n"
        "MortalCoil.exe\n"
        "MortalCoil.exe row col [--special component-id]\n"
        "\n"
    );
}

bool readArguments(int argc, char* argv[], int* row, int* col, int* firstComponentId)
{
    if (argc == 3)
    {
        if (sscanf(argv[1], "%d", row) != 1)
        {
            return false;
        }

        if (sscanf(argv[2], "%d", col) != 1)
        {
            return false;
        }
    }
    else if (argc >= 4)
    {
        // starting from component with given index
        if (sscanf(argv[3], "%d", firstComponentId) != 1)
        {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    clock_t start_time = clock();

    int row = -1, col = -1;
    int firstComponentId = -1;

    if (!readArguments(argc, argv, &row, &col, &firstComponentId))
    {
        printUsage();
        return 0;
    }

    Level level(INPUT_FILENAME);

    TRACE(Colorer::print<RED>("Preprocessing...\n"));

    Analyzer analyzer(&level);
    Solver solver(&level, "output.txt");
    analyzer.preprocess();

    TRACE
    (
        printf("Free cells: %d\n", level.Free);
        printf("Components: %d\n", level.getObstacleCount());

        //level.traceComponent();

        if (level.initialEnds.size() > 0)
        {
            printf("Initial tails: %d\n", level.initialEnds.size());
            for (size_t e = 0; e < level.initialEnds.size(); e++)
            {
                printf("Tail%d: %d %d\n", e + 1, level.initialEnds[e]->getX(), level.initialEnds[e]->getY());
            }
        }
    );

    /*TRACE(Colorer::print<RED>("Analyzing...\n"));

    analyzer.analyzeComponents();

    TRACE(Colorer::print<WHITE>("Max number of solutions: %d\n", Debug::mostSolutions));*/

    TRACE(Colorer::print<RED>("Solving...\n"));

    int firstRow = (row != -1) ? row : 1;
    int firstCol = (col != -1) ? col : 1;
    solver.solve(firstRow, firstCol, firstComponentId);
    
    TRACE(Colorer::print<WHITE>(level.Solved ? "SOLVED\n" : "FAILED TO SOLVE\n"));

#ifdef TRACE_STATISTICS
    printf("Got too many temporary ends %d times!\n", Debug::gotTooManyTemporaryEndsCounter);
    printf("Got invalid next touches %d times!\n", Debug::gotInvalidNextTouchesCounter);
    printf("Got isolated cells %d times!\n", Debug::gotIsolatedCellsCounter);
    printf("Got too many temporary end blocks %d times!\n", Debug::gotTooManyTemporaryEndBlocksCounter);
#endif

    if (level.Solved)
    {
        level.outputToFile(OUTPUT_FILENAME);
    }

    clock_t finish_time = clock();
    printf("%lf\n", double(finish_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}
