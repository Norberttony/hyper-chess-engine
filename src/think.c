
#include "think.h"

// how often the order first (from TT) performs
int orderFirstAttempts = 0;
int orderFirstSuccess = 0;

// search nodes and quiescent search nodes visited
int nodesVisited = 0;

int thinkingTime = -1;
int thinkStart = -1;
int stopThinking = 0;
int currDepth = 0;

#ifdef DEBUG
int cutoffFirst = 0;
int cutoffSecond = 0;
int cutoffThird = 0;
int cutoffAvg = 0;
int cutoffRemaining = 0;
int cutoffRemainingAvg = 0;
#endif

Move currBestMove = 0;

int maxDepth = -1;

int nodeOccurrence[4] = { 0 };


// returns 1 if the engine is still allowed to think and 0 otherwise
void determineThinkAllowance()
{
    if (!(thinkingTime < 0 || (getCurrentTime() - thinkStart) < thinkingTime))
    {
        stopThinking = 1;
    }
    readInput();
}

Move thinkFor(int ms)
{
    thinkStart = getCurrentTime();
    thinkingTime = ms;
    maxDepth = MAX_DEPTH;

    return startThink();
}

Move getBestMove(int depth)
{
    thinkStart = getCurrentTime();
    thinkingTime = -1;
    maxDepth = depth;

    return startThink();
}

// Parameters to set
// - maxDepth: must be set to the depth that will be iteratively searched to.
// - thinkStart: -1 if meant to think indefinitely
// - thinkingTime: must be set
Move startThink(void)
{
    // thinking is back on schedule
    stopThinking = 0;
    orderFirst = 0;

    U64 myHash = zobristHash;

    int totalNodesVisited = 0;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
    printf("info string root-eval %d\n", (toPlay == white ? 1 : -1) * evaluate());
    while (!stopThinking && depth <= maxDepth)
    {
        nodesVisited = 0;

        currDepth = depth;
        think(depth, INT_MIN + 1, INT_MAX - 1);

        // since this is the best move at this depth, it should cause massive cut offs at the next
        // level. A bit of a history heuristic :)
        orderFirst = currBestMove;

        totalNodesVisited += nodesVisited;

        printf("info score ");
        printEval();
        printf(" depth %d nodes %d time %d pv ", depth, totalNodesVisited, getCurrentTime() - thinkStart);
        printPrincipalVariation(depth);
        puts("");
        depth++;
    }

    printf("bestmove %s%s\n", squareNames[get_from(currBestMove)], squareNames[get_to(currBestMove)]);

    if (myHash != zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

#ifdef DEBUG
    int cutoffs = nodeOccurrence[TT_UPPER];
    printf("# of cut-offs: %d\n", cutoffs);
    printf("First move cut-off: %lf%%\n", 100 * (double)cutoffFirst / cutoffs);
    printf("Second move cut-off: %lf%%\n", 100 * (double)cutoffSecond / cutoffs);
    printf("Third move cut-off: %lf%%\n", 100 * (double)cutoffThird / cutoffs);
    printf("On average move cut off after: %lf moves\n", (double)cutoffAvg / cutoffs);
    printf("Cut-off in remaining moves (after hash, killers, captures): %d (%lf%%)\n", cutoffRemaining, 100 * (double)cutoffRemaining / cutoffs);
    printf("Cut-off avg with remaining moves: %lf\n", (double)cutoffRemainingAvg / cutoffRemaining);
#endif

    return currBestMove;
}

int think(int depth, int alpha, int beta)
{
    nodesVisited++;

    if ((nodesVisited & 2047) == 0)
    {
        determineThinkAllowance();
    }

    int nodeType = TT_LOWER;

    int isFromTT = 0;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
#ifdef USE_TRANSPOSITION_TABLE
    if (currDepth != depth && depth >= TT_MIN_DEPTH)
    {
        struct TranspositionEntry* savedEval = getTranspositionTableEntry();
        if (savedEval)
        {
            if (savedEval->depth >= depth)
            {
                if (savedEval->nodeType == TT_EXACT)
                {
                    // determine upper bound for mate score
                    return savedEval->eval - (savedEval->eval >= MATE_SCORE) * (currDepth - depth) + (savedEval->eval <= -MATE_SCORE) * (currDepth - depth);
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

#ifdef DEBUG
    Move orderedFirst = orderFirst;
#endif

    // order most promising moves first
    orderMoves(movelist, size, depth);

    // have at least a move before time runs out
    if (depth == currDepth)
    {
        currBestMove = movelist[0];
    }

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
            eval = INT_MAX - 2 - (currDepth - depth);
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
        if (stopThinking)
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

#ifdef DEBUG
            nodeOccurrence[TT_UPPER]++;
            
            cutoffFirst += i == 0;
            cutoffSecond += i == 1;
            cutoffThird += i == 3;
            cutoffAvg += i + 1;

            if (m != orderedFirst && !is_move_capt(m) && killer_move(depth, 0) != m && killer_move(depth, 1) != m)
            {
                cutoffRemaining++;
                cutoffRemainingAvg += i;
            }
#endif

            // store killer move
            addKillerMove(m, depth);

            // for moves that do not capture...
            if (!is_move_capt(m))
            {
                // update history moves
                int bonus = 600 * depth * depth;

                updateHistory(get_from(m), get_to(m), bonus);

                // all previously searched quiet moves receive a negative score, as they did not
                // provide the desired cut-off. (history maluses)
                // this gives unpromising moves a negative score
                int malusBonus = -bonus;
                for (int j = 0; j < i; j++)
                {
                    Move mj = movelist[j];
                    if (!is_move_capt(mj))
                    {
                        updateHistory(get_from(mj), get_to(mj), malusBonus);
                    }
                }
            }

            return beta;
        }

        if (eval > alpha)
        {
            nodeType = TT_EXACT;
            alpha = eval;
            bestMove = m;

            // set current best move in search only if this is the root node.
            if (depth == currDepth)
            {
                currBestMove = m;
            }

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
    nodesVisited += !accessTT;
    
    if ((nodesVisited & 2047) == 0)
    {
        determineThinkAllowance();
    }

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

        if (stopThinking)
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
