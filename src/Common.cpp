#include "Common.h"

class Level;

namespace Debug
{
    const Cell* INITIAL_CELL = NULL;

    const Level* level = NULL;

    int depth = 0;
    bool traceFlag = false;

#ifdef TRACE_STATISTICS
    int mostSolutions = 0;
    int totalSolutions = 0;
    int gotTooManyTemporaryEndsCounter = 0;
    int gotInvalidNextTouchesCounter = 0;
    int gotIsolatedCellsCounter = 0;
    int gotTooManyTemporaryEndBlocksCounter = 0;
#endif
#ifdef TRACE_SOLUTIONS
    int currentX = -1;
    int currentY = -1;
    int currentDir = -1;
    /*int currentComponent;*/
#endif
}
