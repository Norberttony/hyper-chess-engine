
#include "think.h"

// how often the order first (from TT) performs
U64 orderFirstAttempts = 0;
U64 orderFirstSuccess = 0;

// search nodes and quiescent search nodes visited
U64 nodesVisited = 0;
U64 qNodesVisited = 0;

SearchParams g_searchParams =
{
    .thinkingTime = -1,
    .thinkStart = -1,
    .stopThinking = 0,
    .maxDepth = -1,
    .height = 0
};

// search flags
const uint_fast8_t IS_PV_FLAG = 0x1;
const uint_fast8_t IS_NULL_MOVE_PRUNING_FLAG = 0x2;

#ifdef DEBUG
U64 cutoffFirst = 0;
U64 cutoffSecond = 0;
U64 cutoffThird = 0;
U64 cutoffAvg = 0;
U64 cutoffRemaining = 0;
U64 cutoffRemainingAvg = 0;
#endif

Move currBestMove = 0;

U64 nodeOccurrence[4] = { 0 };


// returns 1 if the engine is still allowed to think and 0 otherwise
void determineThinkAllowance(void)
{
    SearchParams *s = &g_searchParams;
    if (!(s->thinkingTime < 0 || (getCurrentTime() - s->thinkStart) < s->thinkingTime))
    {
        s->stopThinking = 1;
    }
    readInput();
}

Move thinkFor(int ms)
{
    SearchParams *s = &g_searchParams;
    s->thinkStart = getCurrentTime();
    s->thinkingTime = ms;
    s->maxDepth = MAX_DEPTH;

    return startThink();
}

Move getBestMove(int depth)
{
    SearchParams *s = &g_searchParams;
    s->thinkStart = getCurrentTime();
    s->thinkingTime = -1;
    s->maxDepth = depth;

    return startThink();
}

// Parameters to set
// - maxDepth: must be set to the depth that will be iteratively searched to.
// - thinkStart: -1 if meant to think indefinitely
// - thinkingTime: must be set
Move startThink(void)
{
    SearchParams *s = &g_searchParams;

    // thinking is back on schedule
    s->stopThinking = 0;
    orderFirst = 0;

    U64 myHash = g_pos.zobristHash;

    U64 totalNodesVisited = 0ULL;
#ifdef DEBUG
    U64 totalQNodesVisited = 0ULL;
#endif

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
#ifdef DEBUG
    EVAL_DBG_PRINT = 1;
    printf("info string root-eval %d\n", (g_pos.toPlay == white ? 1 : -1) * evaluate());
    EVAL_DBG_PRINT = 0;
#endif
    while (!s->stopThinking && depth <= s->maxDepth)
    {
        nodesVisited = 0ULL;
        qNodesVisited = 0ULL;

        think(depth, -MAX_SCORE, MAX_SCORE, IS_PV_FLAG);

        // since this is the best move at this depth, it should cause massive cut offs at the next
        // level. A bit of a history heuristic :)
        orderFirst = currBestMove;

        totalNodesVisited += nodesVisited;
#ifdef DEBUG
        totalQNodesVisited += qNodesVisited;
#endif

        printf("info score ");
        printEval();
        printf(" depth %d nodes %lld time %d pv ", depth, totalNodesVisited, getCurrentTime() - s->thinkStart);
        printPrincipalVariation(depth);
        puts("");
        depth++;
    }

    printf("bestmove %s%s\n", squareNames[get_from(currBestMove)], squareNames[get_to(currBestMove)]);

    if (myHash != g_pos.zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

#ifdef DEBUG
    U64 cutoffs = nodeOccurrence[TT_UPPER];
    printf("Total nodes: %lld\n", totalNodesVisited);
    printf("Of those, quiescent search nodes: %lld\n", totalQNodesVisited);
    printf("# of cut-offs: %lld\n", cutoffs);
    printf("First move cut-off: %lf%%\n", 100 * (double)cutoffFirst / cutoffs);
    printf("Second move cut-off: %lf%%\n", 100 * (double)cutoffSecond / cutoffs);
    printf("Third move cut-off: %lf%%\n", 100 * (double)cutoffThird / cutoffs);
    printf("On average move cut off after: %lf moves\n", (double)cutoffAvg / cutoffs);
    printf("Cut-off in remaining moves (after hash, killers, captures): %lld (%lf%%)\n", cutoffRemaining, 100 * (double)cutoffRemaining / cutoffs);
    printf("Cut-off avg with remaining moves: %lf\n", (double)cutoffRemainingAvg / cutoffRemaining);
#endif

    return currBestMove;
}

int think(int depth, int alpha, int beta, uint_fast8_t flags)
{
    nodesVisited++;
    int isRoot = g_searchParams.height == 0;
    int distToRoot = g_searchParams.height;

    if ((nodesVisited & 2047ULL) == 0ULL)
    {
        determineThinkAllowance();
    }

    // check for three fold repetition and checkmate
    if (isCheckmate())
    {
        return -(MAX_SCORE - distToRoot);
    }
    else if (!isRoot && getThreefoldFlag() || g_pos.state->halfmove >= DRAW_MOVE_RULE)
    {
        return 0;
    }

    int nodeType = TT_LOWER;

    int isNullMovePruning = flags & IS_NULL_MOVE_PRUNING_FLAG;
    int isPV = flags & IS_PV_FLAG;

    int isFromTT = 0;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
#ifdef USE_TRANSPOSITION_TABLE
    if (!isNullMovePruning && !isRoot && depth >= TT_MIN_DEPTH)
    {
        struct TranspositionEntry* savedEval = getTranspositionTableEntry();
        if (savedEval)
        {
            int savedDepth = TT_getDepth(savedEval->flags);
            int savedNodeType = TT_getNodeType(savedEval->flags);
            int savedVal = TT_getEval(savedEval->flags);
            if (savedDepth >= depth)
            {
                if (savedNodeType == TT_EXACT)
                {
                    // determine upper bound for mate score
                    return savedVal - (savedVal >= MATE_SCORE) * (distToRoot + 1) + (savedVal <= -MATE_SCORE) * (distToRoot + 1);
                }
                else if (savedNodeType == TT_LOWER && alpha >= savedVal)
                {
                    return alpha;
                }
                else if (savedNodeType == TT_UPPER && beta <= savedVal)
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
        return thinkCaptures(alpha, beta, !isNullMovePruning);
    }

    orderFirstAttempts += isFromTT;
    orderFirstSuccess += isFromTT;

    // before generating moves, give the opponent a free move.
    // If we exceed beta, this would mean that my position is so good that the opponent's free move
    // didn't really help them and we can hit a beta cut off.
    int nullDepth = depth - 1 - NULL_MOVE_R;
    if (!isPV && !isNullMovePruning)
    {
        makeNullMove();
        int isInCheck = isAttackingKing();
        if (!isInCheck)
        {
            if (nullDepth < 0)
            {
                nullDepth = 0;
            }
            g_searchParams.height++;
            int nullEval = -think(nullDepth, -beta, -beta + 1, flags | IS_NULL_MOVE_PRUNING_FLAG);
            g_searchParams.height--;
            if (nullEval >= beta)
            {
                makeNullMove();
                return beta;
            }
        }
        makeNullMove();
    }

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist, 0);

    // check for stalemate
    if (size == 0)
    {
        return 0;
    }

#ifdef DEBUG
    Move orderedFirst = orderFirst;
#endif

    // order most promising moves first
    orderMoves(movelist, size, depth);

    // have at least a move before time runs out
    if (isRoot)
    {
        currBestMove = movelist[0];
    }

    int hasLegalMoves = 0;

#ifdef DEBUG
    int mIdx = 0;
#endif

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
        // If this is not the first move OR parent node is not PV, we exclude the PV flag from the
        // rest of the child nodes
        if (hasLegalMoves)
        {
            flags &= ~IS_PV_FLAG;
        }
        hasLegalMoves = 1;

#ifdef DEBUG
        // keep track of move index to avoid factoring in illegal moves as part of the list
        // this is used for cutoff statistics later.
        mIdx++;
#endif

        g_searchParams.height++;
        int eval = -think(depth - 1, -beta, -alpha, flags);
        g_searchParams.height--;

        unmakeMove(m);

        // make sure we are still allowed to think.
        if (g_searchParams.stopThinking)
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
            if (!isNullMovePruning)
            {
                writeToTranspositionTable(depth, beta, m, TT_UPPER);
            }
#endif

#ifdef DEBUG
            nodeOccurrence[TT_UPPER]++;
            
            cutoffFirst += mIdx == 0;
            cutoffSecond += mIdx == 1;
            cutoffThird += mIdx == 3;
            cutoffAvg += mIdx + 1;

            if (m != orderedFirst && !is_move_capt(m) && killer_move(depth, 0) != m && killer_move(depth, 1) != m)
            {
                cutoffRemaining++;
                cutoffRemainingAvg += mIdx;
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
            if (isRoot)
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

#ifdef DEBUG
    nodeOccurrence[nodeType]++;
#endif

    // save this entry in the transposition table
#ifdef USE_TRANSPOSITION_TABLE
    if (!isNullMovePruning)
    {
        writeToTranspositionTable(depth, alpha, bestMove, nodeType);
    }
#endif

    // return best evaluation
    return alpha;
}

int thinkCaptures(int alpha, int beta, int accessTT)
{
    // leaf node is ignored in count of qNodes visited
    nodesVisited += !accessTT;
#ifdef DEBUG
    qNodesVisited += !accessTT;
#endif
    
    if ((nodesVisited & 2047ULL) == 0ULL)
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

        g_searchParams.height++;
        int eval = -thinkCaptures(-beta, -alpha, 0);
        g_searchParams.height--;

        unmakeMove(m);

        if (g_searchParams.stopThinking)
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
