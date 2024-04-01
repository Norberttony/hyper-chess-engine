
#include "transposition-table.h"

struct TranspositionEntry transpositionTable[TRANSPOSITION_TABLE_ENTRIES + 1];

int depthHits[100];
int TT_hits = 0;
int TT_misses = 0;
int TT_misses_type1 = 0;

int TT_entries_filled = 0;
int TT_entries_overwritten = 0;
int TT_nowrites = 0;

void writeToTranspositionTable(U64 zobristHash, int depth, int eval, Move bestMove, int nodeType)
{
    int isOverwrite = transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES].zobristHash != 0;
    TT_entries_overwritten += isOverwrite;
    TT_entries_filled += !isOverwrite;
    transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES] = (struct TranspositionEntry){ zobristHash, depth, eval, bestMove, nodeType };
}
