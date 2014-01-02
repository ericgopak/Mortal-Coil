#include "Common.h"
#include "Colorer.h"
#include "Cell.h"
#include "Obstacle.h"
#include "Component.h"
#include "Analyzer.h"

// Optimized: delete reverse edge only
//void Disconnect(Cell* cell, int dir)
//{
//    //if (cell->next & P[dir])
//    if (cell->Next[dir]->isFree())
//    {        
//        cell->next &= ~P[dir];
//        int &mask = cell->Next[dir]->next; // Neighbours mask of the neighbour
//        mask &= ~P[dir ^ 2];
//        if (Bits[mask] <= 1 && cell->Next[dir]->isFree()) Ends++;
//    }
//}
//
//// Reverse of the Disconnect function
//void Reconnect(Cell* cell, int dir)
//{
//    //if (cell->next & P[dir])
//    if (cell->Next[dir]->isFree())
//    {        
//        cell->next |= P[dir];
//        int &mask = cell->Next[dir]->next; // Neighbour mask of the neighbour
//        if (Bits[mask] <= 1 && cell->Next[dir]->isFree()) Ends--;
//        mask |= P[dir ^ 2];
//    }
//}

namespace Debug
{
    int depth = 0;
    bool traceFlag = false;
}
