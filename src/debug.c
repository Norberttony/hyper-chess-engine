#include "debug.h"
#include <string.h>
#include <stdio.h>
#include "position-defines.h"
#include "move.h"
#include "transposition-table.h"
#include "evaluate-defines.h"
#include "evaluate.h"

static DebugStats debug = { 0 };
int rootDepth = 0;

void printNodeCounts(void);
void printNMP(void);
void printPieceCutoffs(void);
void printPieceCutoffHeatmaps(void);
void printTT(void);

void count_print(void)
{
    printNodeCounts();
    printNMP();
    printPieceCutoffs();
    // printPieceCutoffHeatmaps();
    printTT();
    printf("\n");
}

void printNodeCounts(void)
{
    U64 cutNodes = debug.nodeOccurrences[TT_UPPER];
    U64 allNodes = debug.nodeOccurrences[TT_LOWER];
    U64 pvNodes = debug.nodeOccurrences[TT_EXACT];

    printf("\n");
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

void printNMP(void)
{
    // display NMP success rate
    printf("NMP success rates w/ Beta Margin (BM):\n");
    for (int i = MAX_DEPTH; i >= 0; i--)
    {
        long long int successes = debug.NMP_successes[i];
        long long int tries = debug.NMP_tries[i];

        if (tries == 0)
        {
            continue;
        }

        long long int bm_success = debug.NMP_totalBetaMarginSucc[i];
        long long int bm_fail = debug.NMP_totalBetaMarginFail[i];

        double rate = (double)(100 * successes) / tries;
        double bm_successAvg = (double)bm_success / tries;
        double bm_failAvg = (double)bm_fail / tries;

        printf("(%d) %lld / %lld (%.3f%%) BM success (%.2f) fail (%.2f)\n",
            i, successes, tries, rate, bm_successAvg, bm_failAvg);
    }
    printf("\n");
}

void printPieceCutoffs(void)
{
    puts("Cutoffs by piece types:");
    for (int i = straddler; i <= king; i++)
    {
        U64 quietCutoffs = debug.cutoffsPType[0][i];
        U64 captCutoffs = debug.cutoffsPType[1][i];

        U64 quietIdx = debug.cutoffsPTypeIdx[0][i];
        U64 captIdx = debug.cutoffsPTypeIdx[1][i];

        U64 cutoffs = quietCutoffs + captCutoffs;
        U64 cutoffsIdx = quietIdx + captIdx;
        printf("%c: %lld avg idx: %.3f\n", pieceFEN[i], cutoffs, (double)cutoffsIdx / cutoffs);
        printf(" c %lld avg idx: %.3f\n", captCutoffs, (double)captIdx / captCutoffs);
        printf(" q %lld avg idx: %.3f\n", quietCutoffs, (double)quietIdx / quietCutoffs);
        printf("\n");
    }
    printf("\n");
}

void printPieceCutoffHeatmaps(void)
{
    // print piece cutoff heat maps
    for (int i = straddler; i <= king; i++)
    {
        printf("Piece %c:\n", pieceFEN[i]);
        for (int j = 0; j < 64; j++)
        {
            printf("%8.6lf ", (double)debug.cutoffHeatmaps[i][j] / debug.cutoffHeatmapFreq[i][j]);
            if ((j + 1) % 8 == 0)
            {
                printf("\n");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void printTT(void)
{
    U64 reads = debug.TT_hits + debug.TT_misses;
    // print TT stats
    puts("Transposition Table stats:");
    printf("%lld writes, %lld collisions\n", debug.TT_writes, debug.TT_collisions);
    printf("%lld reads, %lld hits, %lld misses\n", reads, debug.TT_hits, debug.TT_misses);
    printf("Hit rate: %.3lf%%\n", (double)(100 * debug.TT_hits) / reads);
    printf("Current usage: %lld / %d (%.3lf%%)\n",
        debug.TT_entriesUsed, TT_entries, (double)(100 * debug.TT_entriesUsed) / (TT_BUCKETS * TT_entries));
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

// flips the square given the square and the piece type.
// different pieces have a different symmetry with their counterpart.
static inline int debug_getSq(int to, int t)
{
    if (t != immobilizer && t != coordinator)
    {
        // handle symmetry across horizontal (all of the pieces here are aligned in the startpos)
        return reflectSq(to);
    }
    else
    {
        // handle symmetry across y = x
        return flipSq(to);
    }
}

// !! Should not be run for nonquiescent nodes!
void count_move(Move m)
{
    debug.totalMoves++;

    // for heatmaps
    if (!is_move_capt(m))
    {
        int t = get_type(m);
        int to = get_to(m);
        if (g_pos.toPlay == white)
        {
            to = debug_getSq(to, t);
        }
        debug.cutoffHeatmapFreq[t][to]++;
    }
}

void count_betaCutoff(int moveIdx, Move move)
{
    debug.cutoffs[moveIdx]++;
    debug.cutoffsPType[is_move_capt(move)][get_type(move)]++;
    debug.cutoffsPTypeIdx[is_move_capt(move)][get_type(move)] += moveIdx;

    // record occurrence on heatmap
    if (!is_move_capt(move))
    {
        // determine to square
        int to = get_to(move);
        int t = get_type(move);
        if (g_pos.toPlay == black)
        {
            to = debug_getSq(to, t);
        }
        debug.cutoffHeatmaps[t][to]++;
    }
}

void count_nodeVisited(int isQuiescent)
{
    debug.nodesVisited++;
    debug.qNodesVisited += isQuiescent;
    debug.nodesPerDepth[rootDepth] += !isQuiescent;
}

void count_NMP(int success, int depth, int beta)
{
    debug.NMP_successes[depth] += success;
    debug.NMP_tries[depth]++;
    
    if (beta < MATE_SCORE)
    {
        int betaMargin = beta - evaluate();
        if (success)
        {
            debug.NMP_totalBetaMarginSucc[depth] += betaMargin;
        }
        else
        {
            debug.NMP_totalBetaMarginFail[depth] += betaMargin;
        }
    }
}

void count_TT_write(int isCollision)
{
    debug.TT_collisions += isCollision;
    debug.TT_entriesUsed += !isCollision;
    debug.TT_writes++;
}

void count_TT_read(int isHit)
{
    debug.TT_hits += isHit;
    debug.TT_misses += !isHit;
}

void count_TT_clear(void)
{
    // since the TT has been cleared, 0 entries are ued.
    debug.TT_entriesUsed = 0;
}

void count_nodeType(int nodeType)
{
    debug.nodeOccurrences[nodeType]++;
}
