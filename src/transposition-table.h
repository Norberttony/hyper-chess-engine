#ifndef TRANSPOSITION_TABLE_HEADER
#define TRANSPOSITION_TABLE_HEADER

#include "defines.h"
#include "move.h"

#include <string.h>

// must be a power of 2 minus 1
#define TRANSPOSITION_TABLE_ENTRIES 16777215

#define TT_EXACT 1
#define TT_LOWER 2
#define TT_UPPER 3

struct TranspositionEntry
{
    U64 zobristHash; // to-do: I can store the first half of the zobrist hash and combine it with the table index
    int depth;
    int eval;
    Move bestMove;
    int nodeType;
};

extern struct TranspositionEntry transpositionTable[TRANSPOSITION_TABLE_ENTRIES + 1];

extern int depthHits[100];

extern int TT_hits;
extern int TT_misses;
extern int TT_misses_type1;

extern int TT_entries_filled;
extern int TT_entries_overwritten;
extern int TT_nowrites;

void writeToTranspositionTable(U64 zobristHash, int depth, int eval, Move bestMove, int nodeType);

#endif