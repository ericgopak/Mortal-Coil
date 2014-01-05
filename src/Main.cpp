/****************************************************************************************************
    Solution to the problem "Mortal Coil" at http://www.hacker.org/coil
    Author: Eric Gopak

# Ideas:
    o Stop if there is isolated exit (check if mask contains isolated bit, like xxx010xxx)
    o Use Pits (level->Ends) as one-sized temporaryEndBlock
    o Force Simulator::backtrack() visit every cell only ONCE!
    o Pruning: if new exit got blocked - check if there are at least 2 exits left (if it's not the very last component)
    o Redefine 'exits' to be the ones that cannot be blocked during solve()
    o Disallow ANY 'tails' in analysis solutions

# Optimization ideas:
    o Rewrite std::set with std::unordered_set. Better with std::vector. Best with C-style array.
    o Rewrite STL containers with custom ones (maybe just replace std::map with std::unordered_map)
    o Make use of SSE/SSE2 instructions
    o Play around with compiler flags
    o Try compiling with GCC, Intel C++, Clang compilers
    o see http://msdn.microsoft.com/en-us/library/y0dh78ez%28VS.80%29.aspx

# Figure out how to:
    o filter solutions if pits appear (exits are split into separated sets)

# Notes
    o No SPECIAL components in following levels: 24, 28
    o Interesting largest component in Level 28 (2 portals to itself)

****************************************************************************************************/

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
        if (level.getTemporaryEndBlocks().size() > 0)
        {
            printf("Initial end blocks: %d\n", level.getTemporaryEndBlocks().size());
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
