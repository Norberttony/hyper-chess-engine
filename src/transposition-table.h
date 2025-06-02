#ifndef TRANSPOSITION_TABLE_HEADER
#define TRANSPOSITION_TABLE_HEADER

#include "defines.h"
#include "move.h"
#include "make-unmake.h"

#include <limits.h>
#include <string.h>

#ifdef WEB
#define TT_SIZE_MB 64
#else
#define TT_SIZE_MB 128
#endif

#define TT_ENTRIES TT_SIZE_MB * 1000000 / sizeof(struct TranspositionEntry)

#define TT_EXACT 1
#define TT_LOWER 2
#define TT_UPPER 3

// smallest depth allowed to use the transposition table
#define TT_MIN_DEPTH 0

#define TT_getNodeType(flags) (flags & TT_nodeTypeMask)
#define TT_getDepth(flags) ((flags & TT_depthMask) >> 2)
#define TT_getEvalSign(flags) ((flags & TT_evalSignMask) > 0)
#define TT_getEvalVal(flags) ((flags & TT_evalValueMask) >> 10)
#define TT_getEval(flags) (-(2 * TT_getEvalSign(flags) - 1) * TT_getEvalVal(flags))


struct TranspositionEntry
{
    U64 zobristHash; // to-do: I can store the first half of the zobrist hash and combine it with the table index
    Move bestMove;
    uint32_t flags;
};

// statistics related to the transposition table.
extern int TT_misses;
extern int TT_hits;
extern int TT_overwrites;
extern int TT_writes;

extern const int TT_nodeTypeMask;
extern const int TT_depthMask;
extern const int TT_evalSignMask;
extern const int TT_evalValueMask;

// [0] is replace by depth and [1] is always replace
extern struct TranspositionEntry transpositionTable[TT_ENTRIES / 2][2];

// returns either a TT entry that contains the first move of the PV or NULL if there is no such entry
struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth);

// returns either a TT entry that contains cached data about the position's evaluation or NULL if
// there is no such entry.
struct TranspositionEntry* getTranspositionTableEntry(void);

void writeToTranspositionTable(int depth, int eval, Move bestMove, int nodeType);

// prints the evaluation of the position based on the TT
void printEval(void);

// prints the sequence of moves that is considered best play.
void printPrincipalVariation(int depth);

#endif