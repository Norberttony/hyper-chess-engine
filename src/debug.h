#ifndef DEBUG_H
#define DEBUG_H

#include <string.h>
#include <stdint.h>

#include "move.h"
#include "transposition-table.h"
#include "evaluate.h"

typedef struct DebugStats
{
    U64 nodesVisited;
    U64 qNodesVisited;
    U64 totalMoves;
    // total number of nodes searched given depth (across all searches).
    // does not include quiescent nodes! those should be handled separately.
    U64 nodesPerDepth[MAX_DEPTH];
    // how much each depth has occurred (ie. how many times has the engine searched at depth 1?)
    U64 depthCounts[MAX_DEPTH];
    
    U64 cutoffs[MAX_MOVES];
    U64 cutoffsPType[2][8];
    U64 cutoffsPTypeIdx[2][8];
    U64 cutoffHeatmaps[8][64];
    U64 cutoffHeatmapFreq[8][64];
    U64 nodeOccurrences[4];

    U64 NMP_successes[MAX_DEPTH];
    U64 NMP_tries[MAX_DEPTH];
    int64_t NMP_totalBetaMarginSucc[MAX_DEPTH];
    int64_t NMP_totalBetaMarginFail[MAX_DEPTH];

    U64 TT_hits;
    U64 TT_misses;
    U64 TT_writes;
    U64 TT_collisions;
    U64 TT_entriesUsed;
} DebugStats;

void count_print(void);
void count_clear(void);

void count_startDepth(int depth);
void count_move(Move m);
void count_betaCutoff(int moveIdx, Move move);
void count_nodeVisited(int isQuiescent);
void count_NMP(int success, int depth, int betaMargin);
void count_TT_write(int isCollision);
void count_TT_read(int isHit);
void count_TT_clear(void);
void count_nodeType(int nodeType);

#endif
