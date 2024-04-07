#ifndef TRANSPOSITION_TABLE_HEADER
#define TRANSPOSITION_TABLE_HEADER

#include "defines.h"
#include "move.h"

#include <string.h>

// must be a power of 2 minus 1
#define TRANSPOSITION_TABLE_ENTRIES 17999987

#define TT_EXACT 1
#define TT_LOWER 2
#define TT_UPPER 3

// smallest depth allowed to use the transposition table
#define TT_MIN_DEPTH 0

struct TranspositionEntry
{
    U64 zobristHash; // to-do: I can store the first half of the zobrist hash and combine it with the table index
    int depth;
    int eval;
    Move bestMove;
    int nodeType;
};

// statistics related to the transposition table.
extern int TT_misses;
extern int TT_hits;
extern int TT_overwrites;
extern int TT_writes;

extern struct TranspositionEntry transpositionTable_depth[TRANSPOSITION_TABLE_ENTRIES];
extern struct TranspositionEntry transpositionTable_always[TRANSPOSITION_TABLE_ENTRIES];

struct TranspositionEntry* getTranspositionTableEntry(int myDepth);
void writeToTranspositionTable(int depth, int eval, Move bestMove, int nodeType);

// returns the evaluation of the position based on the transposition table
int getEval();

#endif