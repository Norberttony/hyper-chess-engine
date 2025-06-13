
#include "think.h"

// how often the order first (from TT) performs
int orderFirstAttempts = 0;
int orderFirstSuccess = 0;

// search nodes and quiescent search nodes visited
int nodesVisited = 0;
int qNodesVisited = 0;

int thinkingTime = -1;
int thinkStart = -1;
int stopThinking = 0;
int currDepth = 0;

// search flags
const uint_fast8_t IS_PV_FLAG = 0x1;
const uint_fast8_t IS_NULL_MOVE_PRUNING_FLAG = 0x2;

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

    U64 myHash = g_pos.zobristHash;

    int totalNodesVisited = 0;
    int totalQNodesVisited = 0;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
#ifdef DEBUG
    EVAL_DBG_PRINT = 1;
    printf("info string root-eval %d\n", (g_pos.toPlay == white ? 1 : -1) * evaluate());
    EVAL_DBG_PRINT = 0;
#endif
    while (!stopThinking && depth <= maxDepth)
    {
        nodesVisited = 0;
        qNodesVisited = 0;

        currDepth = depth;
        think(depth, INT_MIN + 1, INT_MAX - 1, IS_PV_FLAG);

        // since this is the best move at this depth, it should cause massive cut offs at the next
        // level. A bit of a history heuristic :)
        orderFirst = currBestMove;

        totalNodesVisited += nodesVisited;
        totalQNodesVisited += qNodesVisited;

        printf("info score ");
        printEval();
        printf(" depth %d nodes %d time %d pv ", depth, totalNodesVisited, getCurrentTime() - thinkStart);
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
    int cutoffs = nodeOccurrence[TT_UPPER];
    printf("Total nodes: %d\n", totalNodesVisited);
    printf("Quiescent search nodes: %d\n", totalQNodesVisited);
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

int think(int depth, int alpha, int beta, uint_fast8_t flags)
{
    nodesVisited++;

    if ((nodesVisited & 2047) == 0)
    {
        determineThinkAllowance();
    }

    // check for three fold repetition and checkmate
    if (isCheckmate())
    {
        return -(MAX_SCORE - (currDepth - depth));
    }
    else if (currDepth != depth && getThreefoldFlag())
    {
        return 0;
    }

    int nodeType = TT_LOWER;

    int isNullMovePruning = flags & IS_NULL_MOVE_PRUNING_FLAG;

    int isFromTT = 0;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
#ifdef USE_TRANSPOSITION_TABLE
    if (!isNullMovePruning && currDepth != depth && depth >= TT_MIN_DEPTH)
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
                    return savedVal - (savedVal >= MATE_SCORE) * (currDepth - depth + 1) + (savedVal <= -MATE_SCORE) * (currDepth - depth + 1);
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
        return thinkCaptures(alpha, beta, 1);
    }

    orderFirstAttempts += isFromTT;
    orderFirstSuccess += isFromTT;

    // before generating moves, give the opponent a free move.
    // If we exceed beta, this would mean that my position is so good that the opponent's free move
    // didn't really help them and we can hit a beta cut off.
    int nullDepth = depth - 1 - NULL_MOVE_R;
    if (!isNullMovePruning && nullDepth >= 0)
    {
        makeNullMove();
        int isInCheck = isAttackingKing();
        if (!isInCheck)
        {
            int nullEval = -think(nullDepth, -beta, -beta + 1, flags | IS_NULL_MOVE_PRUNING_FLAG);
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

        int eval = -think(depth - 1, -beta, -alpha, flags);

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
            if (!isNullMovePruning)
            {
                writeToTranspositionTable(depth, beta, m, TT_UPPER);
            }
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
            flags |= IS_PV_FLAG;

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
