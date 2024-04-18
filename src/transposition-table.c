
#include "transposition-table.h"

struct TranspositionEntry transpositionTable_depth[TRANSPOSITION_TABLE_ENTRIES];
struct TranspositionEntry transpositionTable_always[TRANSPOSITION_TABLE_ENTRIES];

int TT_misses = 0;
int TT_hits = 0;
int TT_overwrites = 0;
int TT_writes = 0;

struct TranspositionEntry* getTranspositionTableEntryPV(int myDepth)
{
    int index = (int)(zobristHash % TRANSPOSITION_TABLE_ENTRIES);
    struct TranspositionEntry* depthEntry = transpositionTable_depth + index;
    struct TranspositionEntry* alwaysEntry = transpositionTable_always + index;

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


struct TranspositionEntry* getTranspositionTableEntry(int myDepth)
{
    int index = (int)(zobristHash % TRANSPOSITION_TABLE_ENTRIES);
    struct TranspositionEntry* depthEntry = transpositionTable_depth + index;
    struct TranspositionEntry* alwaysEntry = transpositionTable_always + index;

    // get the first hit and use that as the evaluation.
    // note: this does not prevent search instability.
    if (depthEntry->depth >= myDepth && depthEntry->zobristHash == zobristHash)
    {
        TT_hits++;
        return depthEntry;
    }
    else if (alwaysEntry->depth >= myDepth && alwaysEntry->zobristHash == zobristHash)
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
    transpositionTable_always[index] = (struct TranspositionEntry){ zobristHash, depth, eval, bestMove, nodeType }; 

    // only replace if the depth is better
    if (transpositionTable_depth[index].depth < depth)
    {
        transpositionTable_depth[index] = (struct TranspositionEntry){ zobristHash, depth, eval, bestMove, nodeType }; 
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
            printf("(M%+d) ", extract_mate_score(abs(eval)));
        }
        else
        {
            printf("(%+d) ", eval);
        }
    }
    else
    {
        printf("(--) ");
    }
}
