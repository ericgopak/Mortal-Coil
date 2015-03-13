#include "Common.h"

class Level;

namespace Debug
{
    const Level* level = NULL;

    int depth = 0;
    bool traceFlag = false;

    int ctr1 = 0;
    int ctr2 = 0;

#ifdef TRACE_STATISTICS
    int mostSolutions = 0;
    int totalSolutionsCounter = 0;
    int startingSolutionsCounter = 0;
    int nonStartingSolutionsCounter = 0;
    int throughSolutionsCounter = 0;
    int endingSolutionsCounter = 0;
    int gotTooManyTemporaryEndsCounter = 0;
    int gotInvalidNextTouchesCounter = 0;
    int gotIsolatedCellsCounter = 0;
    int gotTooManyTemporaryEndBlocksCounter = 0;
    int avoidedEndingSolutionCounter = 0;
    int similarSolutionsCounter = 0;
#endif
#ifdef TRACE_SOLUTIONS
    int currentX = -1;
    int currentY = -1;
    int currentDir = -1;
    /*int currentComponent;*/
#endif
}
