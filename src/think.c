
#include "think.h"

// how often the order first (from TT) performs
int orderFirstAttempts = 0;
int orderFirstSuccess = 0;

// search nodes and quiescent search nodes visited
int nodesVisited = 0;
int qNodesVisited = 0;

int thinkingTime = -1;
clock_t thinkStart = -1; // to-do: worried about the precision of clock_t
int maxDepth = 0;

int nodeOccurrence[4] = { 0 };


int getThinkAllowance()
{
    return thinkStart == -1 || (float)(clock() - thinkStart) / (float)((clock_t)1) < thinkingTime;
}

Move thinkFor(int time)
{
    // thinking is back on schedule
    thinkStart = clock();
    thinkingTime = time;
    orderFirst = 0;


    U64 myHash = zobristHash;

    int totalNodesVisited = 0;
    int totalQNodesVisited = 0;

    int totalOrderFirstAttempts = 0;
    int totalOrderFirstSuccess = 0;

    int totalNodeOccurrence[4] = { 0 };

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
    Move bestMove = 0;
    while (getThinkAllowance() && depth < MAX_DEPTH)
    {
        nodesVisited = 0;
        qNodesVisited = 0;

        orderFirstAttempts = 0;
        orderFirstSuccess = 0;

        nodeOccurrence[1] = 0;
        nodeOccurrence[2] = 0;
        nodeOccurrence[3] = 0;

        /*
        TT_misses = 0;
        TT_hits = 0;
        TT_overwrites = 0;
        */

        printf("Searching at depth %d\n", depth + 1);
        printf("Order first: ");
        prettyPrintMove(orderFirst);

        Move candidate = getBestMove(++depth);

        /*
        puts("Transposition Table Results");
        printf("Hits: %d\n", TT_hits);
        printf("Misses: %d\n", TT_misses);
        printf("Writes: %d / %d\n", TT_writes, TRANSPOSITION_TABLE_ENTRIES + 1);
        printf("Overwrites: %d / %d\n", TT_overwrites, TT_writes + TT_overwrites);
        */

        // since this is the best move at this depth, it should cause massive cut offs at the next
        // level. A bit of a history heuristic :)
        orderFirst = candidate;

        bestMove = candidate;
        printEval();
        printPrincipalVariation(depth);
        printf("\n");
        printf("Visited %d nodes\n", nodesVisited);
        printf("Visited %d quiescent nodes\n", qNodesVisited);
        printf("Ordered first and was correct %d/%d times\n", orderFirstSuccess, orderFirstAttempts);
        printf("EXACT: %d\nLOWER: %d\nUPPER: %d\n", nodeOccurrence[TT_EXACT], nodeOccurrence[TT_LOWER], nodeOccurrence[TT_UPPER]);
        printf("\n");

        totalNodesVisited += nodesVisited;
        totalQNodesVisited += qNodesVisited;

        totalOrderFirstAttempts += orderFirstAttempts;
        totalOrderFirstSuccess += orderFirstSuccess;

        totalNodeOccurrence[TT_EXACT] += nodeOccurrence[TT_EXACT];
        totalNodeOccurrence[TT_LOWER] += nodeOccurrence[TT_LOWER];
        totalNodeOccurrence[TT_UPPER] += nodeOccurrence[TT_UPPER];
    }

    printf("Total number of nodes visited: %d\n", totalNodesVisited);
    printf("Total number of quiescent nodes visited: %d\n", totalQNodesVisited);
    printf("Total number of order first successes: %d/%d\n", totalOrderFirstSuccess, totalOrderFirstAttempts);
    printf("Total number of node occurrences | EXACT: %d LOWER: %d UPPER: %d\n", totalNodeOccurrence[TT_EXACT], totalNodeOccurrence[TT_LOWER], totalNodeOccurrence[TT_UPPER]);

    if (myHash != zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

    printf("Had to stop at depth %d\n", depth);

    // set back to always think
    thinkingTime = -1;

    return bestMove;
}

int think(int depth, int alpha, int beta)
{
    nodesVisited++;

    int nodeType = TT_LOWER;

    int isFromTT = 0;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
#ifdef USE_TRANSPOSITION_TABLE
    if (depth >= TT_MIN_DEPTH)
    {
        struct TranspositionEntry* savedEval = getTranspositionTableEntry(depth);
        if (savedEval)
        {
            if (savedEval->depth >= depth)
            {
                if (savedEval->nodeType == TT_EXACT)
                {
                    // determine upper bound for mate score
                    return savedEval->eval - (savedEval->eval >= MATE_SCORE) * (maxDepth - depth) + (savedEval->eval <= -MATE_SCORE) * (maxDepth - depth);
                }
                else if (savedEval->nodeType == TT_LOWER && alpha >= savedEval->eval)
                {
                    return alpha;
                }
                else if (savedEval->nodeType == TT_UPPER && beta <= savedEval->eval)
                {
                    return beta;
                }
            }
            // order based on saved entry
            orderFirst = savedEval->bestMove;
            isFromTT = 1;
        }
    }
#endif

    // perform quiescent search at leaf nodes
    if (depth == 0)
    {
        return thinkCaptures(alpha, beta, 1);
    }

    orderFirstAttempts += isFromTT;
    orderFirstSuccess += isFromTT;

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist, 0);

    if (size == 0)
    {
        return 0; // no moves! don't try sorting
    }

    // order most promising moves first
    orderMoves(movelist, size, depth);

    int hasLegalMoves = 0;

    Move bestMove = 0;
    for (int i = 0; i < size; i++)
    {
        Move m = movelist[i];
        makeMove(m);

        if (isAttackingKing())
        {
            unmakeMove(m);
            continue;
        }
        hasLegalMoves = 1;

        int eval;

        // check for three fold repetition
        int threefold = getThreefoldFlag();

        if (isCheckmate())
        {
            eval = INT_MAX - 1 - (maxDepth - depth);
        }
        else if (threefold)
        {
            eval = 0;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // make sure we are still allowed to think.
        if (!getThinkAllowance())
        {
            return beta;
        }

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            // store the move that caused the cut off
#ifdef USE_TRANSPOSITION_TABLE
            writeToTranspositionTable(depth, beta, m, TT_UPPER);
#endif
            nodeOccurrence[TT_UPPER]++;
            
            // store killer move
            int isStored = killerMoves[depth][0] == m;
            killerMoves[depth][1] = !isStored * killerMoves[depth][0] + isStored * killerMoves[depth][1];
            killerMoves[depth][0] = !isStored * m + isStored * killerMoves[depth][0];

            return beta;
        }

        if (eval > alpha)
        {
            nodeType = TT_EXACT;
            alpha = eval;
            bestMove = m;

            orderFirstSuccess -= isFromTT;
            isFromTT = 0;
        }
    }

    // if there are no legal moves in this position, it must be a stalemate.
    if (!hasLegalMoves)
    {
        return 0;
    }

    nodeOccurrence[nodeType]++;

    // save this entry in the transposition table
#ifdef USE_TRANSPOSITION_TABLE
    writeToTranspositionTable(depth, alpha, bestMove, nodeType);
#endif

    // return best evaluation
    return alpha;
}

int thinkCaptures(int alpha, int beta, int accessTT)
{
    // leaf node is ignored in count of qNodes visited
    qNodesVisited += !accessTT;

    // not capturing might be better
    int eval = evaluate();
    if (eval >= beta)
    {
#ifdef USE_TRANSPOSITION_TABLE
        if (accessTT)
        {
            writeToTranspositionTable(0, beta, 0, TT_UPPER);
        }
#endif
        return beta;
    }
    if (eval > alpha)
    {
        alpha = eval;
    }

    // generate only captures
    Move movelist[MAX_CAPTURES];
    int size = generateMoves((Move*)movelist, 1);

    if (size == 0)
    {
        return alpha; // no moves!
    }

    // determine most promising moves
    orderMoves(movelist, size, -1);

    int nodeType = TT_LOWER;

    for (int i = 0; i < size; i++)
    {
        Move m = movelist[i];
        makeMove(m);

        if (isAttackingKing())
        {
            unmakeMove(m);
            continue;
        }

        int eval = -thinkCaptures(-beta, -alpha, 0);

        unmakeMove(m);

        if (!getThinkAllowance())
        {
            return beta;
        }

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
#ifdef USE_TRANSPOSITION_TABLE
            if (accessTT)
            {
                writeToTranspositionTable(0, beta, 0, TT_UPPER);
            }
#endif
            return beta;
        }

        if (eval > alpha)
        {
            nodeType = TT_EXACT;
            alpha = eval;
        }
    }

#ifdef USE_TRANSPOSITION_TABLE
    if (accessTT)
    {
        writeToTranspositionTable(0, alpha, 0, nodeType);
    }
#endif

    // return best evaluation
    return alpha;
}

Move getBestMove(int depth)
{
    maxDepth = depth;
    nodesVisited++;

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist, 0);

    if (size == 0)
    {
        return 0; // no moves!
    }

    // determine most promising moves
    orderMoves(movelist, size, depth);

    int alpha = INT_MIN + 1;
    int beta = INT_MAX - 1;
    Move bestMove = movelist[0];

    printf("Ordered first ");
    prettyPrintMove(bestMove);

    for (int i = 0; i < size; i++)
    {
        Move m = movelist[i];
        makeMove(m);

        if (isAttackingKing())
        {
            unmakeMove(m);
            continue;
        }

        // try to maximize value
        int eval;

        // check for three fold repetition
        int threefold = getThreefoldFlag();

        if (isCheckmate())
        {
            eval = INT32_MAX - 1 - (maxDepth - depth);
        }
        else if (threefold)
        {
            eval = 0;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // make sure we are still allowed to think.
        // if not, throw in the best move we've got!
        if (!getThinkAllowance())
        {
            printf("depth %d: evaluating the position as %d\n", depth, alpha);
            return bestMove;
        }

        if (eval > alpha)
        {
            printf("Overthrew last best move with eval %d, new best move with eval %d is ", alpha, eval);
            prettyPrintMove(m);
            puts("");
            alpha = eval;
            bestMove = m;
        }
    }

    printf("depth %d: evaluating the position as %d\n", depth, alpha);

#ifdef USE_TRANSPOSITION_TABLE
    writeToTranspositionTable(depth, alpha, bestMove, TT_EXACT);
#endif

    return bestMove;
}
