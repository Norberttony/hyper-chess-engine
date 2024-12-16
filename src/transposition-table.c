
#include "transposition-table.h"

struct TranspositionEntry transpositionTable[TRANSPOSITION_TABLE_ENTRIES][2];

int TT_misses = 0;
int TT_hits = 0;
int TT_overwrites = 0;
int TT_writes = 0;

struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth)
{
    int index = (int)(zobristHash % TRANSPOSITION_TABLE_ENTRIES);
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (depthEntry->depth >= myDepth && depthEntry->zobristHash == zobristHash && depthEntry->nodeType == TT_EXACT)
    {
        return depthEntry;
    }
    else if (alwaysEntry->depth >= myDepth && alwaysEntry->zobristHash == zobristHash && alwaysEntry->nodeType == TT_EXACT)
    {
        return alwaysEntry;
    }

    return NULL;
}


struct TranspositionEntry* getTranspositionTableEntry(void)
{
    int index = (int)(zobristHash % TRANSPOSITION_TABLE_ENTRIES);
    struct TranspositionEntry* depthEntry = &transpositionTable[index][0];
    struct TranspositionEntry* alwaysEntry = &transpositionTable[index][1];

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (depthEntry->zobristHash == zobristHash)
    {
        TT_hits++;
        return depthEntry;
    }
    else if (alwaysEntry->zobristHash == zobristHash)
    {
        TT_hits++;
        return alwaysEntry;
    }

    TT_misses++;

    return NULL;
}

void writeToTranspositionTable(int depth, int eval, Move bestMove, int nodeType)
{
    if (depth < TT_MIN_DEPTH)
    {
        return;
    }

    int index = (int)(zobristHash % TRANSPOSITION_TABLE_ENTRIES);

    //TT_overwrites += transpositionTable_always[index].zobristHash != 0;
    //TT_writes += transpositionTable_always[index].zobristHash == 0;

    // always replace any entry here
    transpositionTable[index][1] = (struct TranspositionEntry){ zobristHash, depth, eval, bestMove, nodeType }; 

    // only replace if the depth is better
    if (transpositionTable[index][0].depth < depth)
    {
        transpositionTable[index][0] = (struct TranspositionEntry){ zobristHash, depth, eval, bestMove, nodeType }; 
    }
}

void printEval()
{
    struct TranspositionEntry* entry = getTranspositionTableEntryPV(0);
    if (entry)
    {
        int eval = entry->eval;
        if (eval >= MATE_SCORE || eval <= -MATE_SCORE)
        {
            printf("mate %+d ", (toPlay == black ? -1 : 1) * (eval < 0 ? -1 : 1) * extract_mate_score(abs(eval)));
        }
        else
        {
            printf("cp %+d", (toPlay == black ? -1 : 1) * eval);
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

    // get entry from transposition table
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
