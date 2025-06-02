
#include "transposition-table.h"

struct TranspositionEntry transpositionTable[TT_ENTRIES / 2][2];

int TT_misses = 0;
int TT_hits = 0;
int TT_overwrites = 0;
int TT_writes = 0;

const int TT_nodeTypeMask = 0x3;
const int TT_depthMask = 0x1FC;
const int TT_evalSignMask = 0x200;
const int TT_evalValueMask = 0xFFFFFC00;


struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth)
{
    int index = (int)(g_pos.zobristHash % (TT_ENTRIES / 2));
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (TT_getDepth(depthEntry->flags) >= myDepth && depthEntry->zobristHash == g_pos.zobristHash && TT_getNodeType(depthEntry->flags) == TT_EXACT)
    {
        return depthEntry;
    }
    else if (TT_getDepth(alwaysEntry->flags) >= myDepth && alwaysEntry->zobristHash == g_pos.zobristHash && TT_getNodeType(alwaysEntry->flags) == TT_EXACT)
    {
        return alwaysEntry;
    }

    return NULL;
}

struct TranspositionEntry* getTranspositionTableEntry(void)
{
    int index = (int)(g_pos.zobristHash % (TT_ENTRIES / 2));
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (depthEntry->zobristHash == g_pos.zobristHash)
    {
        // TT_hits++;
        return depthEntry;
    }
    else if (alwaysEntry->zobristHash == g_pos.zobristHash)
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

    int index = (int)(g_pos.zobristHash % (TT_ENTRIES / 2));

    // TT_overwrites += transpositionTable[index][1].zobristHash != 0;
    // TT_writes++;

    // always replace any entry here
    uint32_t flags = ((uint32_t)abs(eval) << 10) | ((eval < 0) << 9) | (depth << 2) | nodeType;
    transpositionTable[index][1] = (struct TranspositionEntry){ g_pos.zobristHash, bestMove, flags }; 

    // only replace if the depth is better
    if (TT_getDepth(transpositionTable[index][0].flags) < depth)
    {
        transpositionTable[index][0] = (struct TranspositionEntry){ g_pos.zobristHash, bestMove, flags }; 
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
            printf("mate %+d ", (g_pos.toPlay == black ? -1 : 1) * (eval < 0 ? -1 : 1) * extract_mate_score(abs(eval)));
        }
        else
        {
            printf("cp %+d", (g_pos.toPlay == black ? -1 : 1) * eval);
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
