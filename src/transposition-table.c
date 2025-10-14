
#include "transposition-table.h"

struct TranspositionEntry transpositionTable[MAX_TT_ENTRIES][TT_BUCKETS] = { 0 };

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
#ifdef DEBUG
        count_TT_read(1);
#endif
        return depthEntry;
    }
    else if (alwaysEntry->zobristHash == hash)
    {
#ifdef DEBUG
        count_TT_read(1);
#endif
        return alwaysEntry;
    }

#ifdef DEBUG
    count_TT_read(0);
#endif

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

    // adjust mate scores in the TT
    int ply = g_searchParams.height;
    if (eval >= MATE_SCORE)
    {
        eval += ply;
    }
    else if (eval <= -MATE_SCORE)
    {
        eval -= ply;
    }

#ifdef DEBUG
    count_TT_write(transpositionTable[index][1].zobristHash != 0);
#endif

    // always replace any entry here
    uint32_t flags = ((uint32_t)abs(eval) << 10) | ((eval < 0) << 9) | (depth << 2) | nodeType;
    transpositionTable[index][1] = (struct TranspositionEntry){ hash, bestMove, flags }; 

    // only replace if the depth is better
    if (TT_getDepth(transpositionTable[index][0].flags) < depth)
    {
#ifdef DEBUG
        count_TT_write(transpositionTable[index][0].zobristHash != 0);
#endif
        transpositionTable[index][0] = (struct TranspositionEntry){ hash, bestMove, flags }; 
    }
}

void printEval(int eval)
{
    if (eval >= MATE_SCORE || eval <= -MATE_SCORE)
    {
        printf("mate %+d", (eval < 0 ? -1 : 1) * extract_mate_score(abs(eval)));
    }
    else
    {
        printf("cp %+d", eval);
    }
}

void printEval_TT(void)
{
    struct TranspositionEntry* entry = getTranspositionTableEntryPV(0);
    if (entry)
    {
        printEval(TT_getEval(entry->flags));
    }
    else
    {
        printf("(--)");
    }
}

void printPrincipalVariation(int depth, int maxDepth)
{
    // base case
    // this is the last safety net in case we hit an unusual case and there's an infinite loop
    // in this function with nothing to break it.
    if (maxDepth == 0)
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

        // if we repeated three times already, then that's it. it's a draw.
        // IF somehow we get a repetition that is longer than the repetition circular buffer can
        // hold, then when the halfmove starts being factored into the TT hash (because of 50 move
        // rule) we will break out of the loop.
        if (getNumberOfRepeats() >= 3)
        {
            unmakeMove(entry->bestMove);
            return;
        }

        printPrincipalVariation(TT_getDepth(entry->flags) - 1, maxDepth - 1);
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
