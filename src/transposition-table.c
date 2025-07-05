
#include "transposition-table.h"

struct TranspositionEntry transpositionTable[MAX_TT_ENTRIES][2] = { 0 };

int TT_misses = 0;
int TT_hits = 0;
int TT_overwrites = 0;
int TT_writes = 0;

const int TT_nodeTypeMask = 0x3;
const int TT_depthMask = 0x1FC;
const int TT_evalSignMask = 0x200;
const int TT_evalValueMask = 0xFFFFFC00;

// by default set the TT size to 32MB
int TT_entries = TT_GET_NUMBER_OF_ENTRIES(32);


// Courtesy of https://www.talkchess.com/forum/viewtopic.php?t=60264
// Returns the hash of the position, sometimes factoring in the fifty move rule
static inline __attribute__((always_inline)) U64 getTranspositionHash(void)
{
    U64 hash = g_pos.zobristHash;

    int hm = g_pos.state->halfmove;
    if (hm >= DRAW_MOVE_RULE - ZOBRIST_HASH_COUNT_HALFMOVE)
    {
        if (hm > DRAW_MOVE_RULE)
        {
            hm = DRAW_MOVE_RULE;
        }
        hash ^= zobristHashes_halfmoves[hm - (DRAW_MOVE_RULE - ZOBRIST_HASH_COUNT_HALFMOVE)];
    }

    return hash;
}

struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth)
{
    U64 hash = getTranspositionHash();

    int index = (int)(hash % TT_entries);
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (TT_getDepth(depthEntry->flags) >= myDepth && depthEntry->zobristHash == hash && TT_getNodeType(depthEntry->flags) == TT_EXACT)
    {
        return depthEntry;
    }
    else if (TT_getDepth(alwaysEntry->flags) >= myDepth && alwaysEntry->zobristHash == hash && TT_getNodeType(alwaysEntry->flags) == TT_EXACT)
    {
        return alwaysEntry;
    }

    return NULL;
}

struct TranspositionEntry* getTranspositionTableEntry(void)
{
    U64 hash = getTranspositionHash();

    int index = (int)(hash % TT_entries);
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (depthEntry->zobristHash == hash)
    {
        // TT_hits++;
        return depthEntry;
    }
    else if (alwaysEntry->zobristHash == hash)
    {
        // TT_hits++;
        return alwaysEntry;
    }

    // TT_misses++;

    return NULL;
}

void writeToTranspositionTable(int depth, int eval, Move bestMove, int nodeType)
{
    if (depth < TT_MIN_DEPTH)
    {
        return;
    }

    U64 hash = getTranspositionHash();

    int index = (int)(hash % TT_entries);

    // TT_overwrites += transpositionTable[index][1].zobristHash != 0;
    // TT_writes++;

    // always replace any entry here
    uint32_t flags = ((uint32_t)abs(eval) << 10) | ((eval < 0) << 9) | (depth << 2) | nodeType;
    transpositionTable[index][1] = (struct TranspositionEntry){ hash, bestMove, flags }; 

    // only replace if the depth is better
    if (TT_getDepth(transpositionTable[index][0].flags) < depth)
    {
        transpositionTable[index][0] = (struct TranspositionEntry){ hash, bestMove, flags }; 
    }
}

void printEval(void)
{
    struct TranspositionEntry* entry = getTranspositionTableEntryPV(0);
    if (entry)
    {
        int eval = TT_getEval(entry->flags);
        if (eval >= MATE_SCORE || eval <= -MATE_SCORE)
        {
            printf("mate %+d ", (eval < 0 ? -1 : 1) * extract_mate_score(abs(eval)));
        }
        else
        {
            printf("cp %+d", eval);
        }
    }
    else
    {
        printf("(--) ");
    }
}

void printPrincipalVariation(int depth)
{
    // base case
    // a leaf node does not have a "best move" tied to it.
    if (depth == 0)
    {
        return;
    }

    // get PV entry from transposition table
    struct TranspositionEntry* entry = getTranspositionTableEntryPV(depth);

    if (entry && entry->bestMove)
    {
        printMove(entry->bestMove);

        // update the zobrist hash, and keep printing the principal variation.
        makeMove(entry->bestMove);
        printPrincipalVariation(depth - 1);
        unmakeMove(entry->bestMove);
    }
}

void setTranspositionTableSize(int mb)
{
    if (mb > MAX_TT_SIZE_MB)
    {
        mb = MAX_TT_SIZE_MB;
    }
    else if (mb < 1)
    {
        mb = 1;
    }
    TT_entries = TT_GET_NUMBER_OF_ENTRIES(mb);
}
