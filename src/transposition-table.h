#ifndef TRANSPOSITION_TABLE_HEADER
#define TRANSPOSITION_TABLE_HEADER

#include "defines.h"
#include "move.h"
#include "make-unmake.h"

#include <limits.h>
#include <string.h>

#ifdef WEB
#define TRANSPOSITION_TABLE_ENTRIES 899981
#else
#define TRANSPOSITION_TABLE_ENTRIES 17999987
#endif

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

// [0] is replace by depth and [1] is always replace
extern struct TranspositionEntry transpositionTable[TRANSPOSITION_TABLE_ENTRIES][2];

struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth);
struct TranspositionEntry* getTranspositionTableEntry(void);
void writeToTranspositionTable(int depth, int eval, Move bestMove, int nodeType);

// returns the evaluation of the position based on the transposition table
void printEval();

// prints the sequence of moves that is considered best play.
void printPrincipalVariation(int depth);

#endif