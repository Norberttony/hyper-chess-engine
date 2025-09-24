
#include "debug.h"

static DebugStats debug = { 0 };
int rootDepth = 0;


void count_print(void)
{
    U64 cutNodes = debug.nodeOccurrences[TT_UPPER];
    U64 allNodes = debug.nodeOccurrences[TT_LOWER];
    U64 pvNodes = debug.nodeOccurrences[TT_EXACT];

    printf("Total nodes visited: %lld\n", debug.nodesVisited);
    printf("Total quiescent nodes visited: %lld\n", debug.qNodesVisited);
    
    // display percentage of first few cutoffs
    printf("Cutoffs... ");
    for (int i = 0; i < 8; i++)
    {
        U64 cutoffs = debug.cutoffs[i];
        printf("%d: %lld (%.3lf%%); ", i + 1, cutoffs, (double)(100 * cutoffs) / cutNodes);
    }
    printf("\n");

    // display mean branching factor of the main tree (ie. non-quiescent nodes)
    U64 nonterminalNodes = debug.nodesVisited - debug.qNodesVisited;
    printf("Mean branching factor: %.2lf\n", (double)debug.totalMoves / nonterminalNodes);

    // display evolution of average EBF (effective branching factor) between depths
    printf("Average EBF (between depths): ");
    for (int i = 1; i < rootDepth; i++)
    {
        double nodes = (double)debug.nodesPerDepth[i] / debug.depthCounts[i];
        double nextNodes = (double)debug.nodesPerDepth[i + 1] / debug.depthCounts[i + 1];
        printf("(%d: %.2lf) --%.2lf--> ", i, nodes, nextNodes / nodes);
    }
    printf("(%d: %.2lf)\n", rootDepth, (double)debug.nodesPerDepth[rootDepth] / debug.depthCounts[rootDepth]);

    // display node type frequency
    printf("All nodes (%lld) | Cut nodes (%lld) | PV nodes (%lld)\n", allNodes, cutNodes, pvNodes);

    printf("\n");
}

void count_clear(void)
{
    memset(&debug, 0, sizeof(debug));
}

void count_startDepth(int depth)
{
    rootDepth = depth;
    debug.depthCounts[depth]++;
}

// !! Should not be run for nonquiescent nodes!
void count_move(Move m)
{
    debug.totalMoves++;
}

void count_betaCutoff(int moveIdx, Move move)
{
    debug.cutoffs[moveIdx]++;
}

void count_nodeVisited(int isQuiescent)
{
    debug.nodesVisited++;
    debug.qNodesVisited += isQuiescent;
    debug.nodesPerDepth[rootDepth] += !isQuiescent;
}

void count_NMP(int success)
{
}

void count_writeTT(int nodeType, int success)
{
}

void count_nodeType(int nodeType)
{
    debug.nodeOccurrences[nodeType]++;
}
