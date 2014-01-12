/****************************************************************************************************
    Solution to the problem "Mortal Coil" at http://www.hacker.org/coil
    Author: Eric Gopak

# Ideas:
    o Generate starting solutions only before follow() (not during Analysis)
    o Stop if there is isolated exit (check if mask contains isolated bit, like xxx010xxx)
    o Use Pits (level->Ends) as one-sized temporaryEndBlock

# Optimization ideas:
    o Rewrite std::set with std::unordered_set. Better with std::vector. Best with C-style array.
    o Rewrite STL containers with custom ones (maybe just replace std::map with std::unordered_map)
    o Make use of SSE/SSE2 instructions
    o Play around with compiler flags
    o Try compiling with GCC, Intel C++, Clang compilers
    o see http://msdn.microsoft.com/en-us/library/y0dh78ez%28VS.80%29.aspx

# Figure out how to:
    o check if current component is (not) the last one (perhaps store solution fragment lengths + update cellsVisited for every follow()?)
    o efficiently retrieve remaining solutions (O(logN) --> O(1))
    o filter solutions if pits appear (exits are split into separated sets)

# Notes
    o No SPECIAL components in following levels: 24, 28
    o Interesting largest component in Level 28 (2 portals to itself)
    o 'Donut' component in level 17

# Known bugs
    o When starting analysis right in front of an exit it happens to turn sideways

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

    Debug::level = &level;

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

#ifdef TRACE_MAIN_STEPS
    Colorer::print<RED>("Analyzing...\n");
#endif

    analyzer.analyzeComponents();

#ifdef TRACE_STATISTICS
    Colorer::print<WHITE>("Max number of solutions: %d\n", Debug::mostSolutions);
    Colorer::print<WHITE>("Total solutions found: %d\n", Debug::totalSolutionsCounter);
    Colorer::print<WHITE>("Starting solutions found: %d\n", Debug::startingSolutionsCounter);
    Colorer::print<WHITE>("Non-starting solutions found: %d\n", Debug::nonStartingSolutionsCounter);
    Colorer::print<WHITE>("Number of SPECIAL components: %d\n", level.getSpecialComponents().size());
    Colorer::print<WHITE>("Found %d similar solutions!\n", Debug::similarSolutionsCounter);
#endif

#ifdef TRACE_MAIN_STEPS
    Colorer::print<RED>("Solving...\n");
#endif

    int firstRow = (row != -1) ? row : 1;
    int firstCol = (col != -1) ? col : 1;

    solver.solve(firstRow, firstCol, firstComponentId);

#ifdef TRACE_STATISTICS
    /*printf("Got too many temporary ends %d times!\n", Debug::gotTooManyTemporaryEndsCounter);
    printf("Got invalid next touches %d times!\n", Debug::gotInvalidNextTouchesCounter);
    printf("Got isolated cells %d times!\n", Debug::gotIsolatedCellsCounter);
    printf("Got too many temporary end blocks %d times!\n", Debug::gotTooManyTemporaryEndBlocksCounter);*/
    printf("Avoided ending solutions %d times!\n", Debug::avoidedEndingSolutionCounter);
#endif

    if (level.Solved)
    {
#ifdef TRACE_MAIN_STEPS
    Colorer::print<YELLOW>("Solution: x=%d y=%d\n%s\n", level.getSolutionStartX(), level.getSolutionStartY(), level.Answer.c_str());
#endif
        level.outputToFile(OUTPUT_FILENAME);
    }

    clock_t finish_time = clock();
    printf("%lf\n", double(finish_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}
