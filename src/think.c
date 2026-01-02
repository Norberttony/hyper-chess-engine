#include "think.h"

// search flags
const SearchFlags IS_PV_FLAG = 0x1;
const SearchFlags IS_NULL_MOVE_PRUNING_FLAG = 0x2;

// returns 1 if the engine is still allowed to think and 0 otherwise
static inline void determineThinkAllowance(SearchResults* s)
{
    if ((s->nodesVisited & 2047ULL) == 0ULL)
    {
        if (!(s->thinkingTime < 0 || (getCurrentTime() - s->thinkStart) < s->thinkingTime))
        {
            s->stopThinking = 1;
        }
        readInput();
    }
}

void startThink(SearchParams* s, SearchResults* res)
{
    // thinking is back on schedule
    res->stopThinking = 0;
    res->thinkingTime = s->thinkingTime;
    res->thinkStart = s->thinkStart;
    res->bestMove = 0;

    orderFirst = 0;

    U64 myHash = g_pos.zobristHash;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 1;
#ifdef DEBUG
    EVAL_DBG_PRINT = 1;
    printf("info string root-eval %d\n", (g_pos.toPlay == white ? 1 : -1) * evaluate());
    EVAL_DBG_PRINT = 0;
#endif
    while (!res->stopThinking && depth <= s->maxDepth)
    {
#ifdef DEBUG
        count_startDepth(depth);
#endif
        U64 prevNodes = res->nodesVisited;
        int eval = think(depth, -MAX_SCORE, MAX_SCORE, res, IS_PV_FLAG);
        U64 nodesVisitedDepth = res->nodesVisited - prevNodes;

        // handling checkmate (or stalemate) at the root
        if (depth > 0 && nodesVisitedDepth == 1ULL)
        {
            printf("info score ");
            printEval(eval);
            puts(" depth 0");
            break;
        }

        // since this is the best move at this depth, it will be ordered first
        // (as it's likely still the first move of the PV).
        orderFirst = res->bestMove;

        printf("info score ");
        printEval_TT();
        printf(" depth %d nodes %lld time %d pv ",
            depth, res->nodesVisited, getCurrentTime() - s->thinkStart);
        saveRepeatTable();
        if (res->stopThinking)
        {
            // at least print part of the current principal variation if
            // prompted to stop thinking if we changed moves at the root but
            // interrupted thinking, the root hasn't had enough time to put in
            // the best move into the TT. So, we forcefully play it here and
            // then play out the PV. This might still give an inaccurate PV
            // (because of missing TT entries) but should be right a good
            // amount of the time.
            printMove(res->bestMove);
            makeMove(res->bestMove);
            printPrincipalVariation(depth - 1, 2 * (depth - 1));
            unmakeMove(res->bestMove);
        }
        else
        {
            printPrincipalVariation(depth, 2 * depth);
        }
        restoreRepeatTable();
        puts("");
        depth++;
    }

    printf("bestmove %s%s\n",
        squareNames[get_from(res->bestMove)], squareNames[get_to(res->bestMove)]);

    if (myHash != g_pos.zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

    return ;
}

int think(int depth, int alpha, int beta, SearchResults* res, SearchFlags flags)
{
    res->nodesVisited++;
    int isRoot = res->height == 0;
    int distToRoot = res->height;

    determineThinkAllowance(res);

    // check for three fold repetition and checkmate
    if (isCheckmate())
    {
        return -(MAX_SCORE - distToRoot);
    }
    else if (!isRoot && (getNumberOfRepeats() >= 2 || g_pos.state->halfmove >= DRAW_MOVE_RULE))
    {
        return 0;
    }

    int nodeType = TT_LOWER;
    int isNullMovePruning = flags & IS_NULL_MOVE_PRUNING_FLAG;
    int isPV = flags & IS_PV_FLAG;

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
                    return savedVal - (savedVal >= MATE_SCORE) * distToRoot + (savedVal <= -MATE_SCORE) * distToRoot;
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
        }
    }
#endif

    // perform quiescent search at leaf nodes
    if (depth == 0)
    {
        return thinkCaptures(alpha, beta, res, !isNullMovePruning);
    }

#ifdef DEBUG
    count_nodeVisited(0);
#endif

    makeNullMove();
    int isInCheck = isAttackingKing();
    makeNullMove();

    // before generating moves, give the opponent a free move.
    // If we exceed beta, this would mean that my position is so good that the opponent's free move
    // didn't really help them. We might get a beta cut off.
    int nullDepth = depth - 1 - NULL_MOVE_R;
    if (!isPV && !isNullMovePruning && evaluate() + 30 - 10 * depth >= beta)
    {
        makeNullMove();
        if (!isInCheck)
        {
            if (nullDepth < 0)
            {
                nullDepth = 0;
            }
            res->height++;
            int nullEval = -think(nullDepth, -beta, -beta + 1, res, flags | IS_NULL_MOVE_PRUNING_FLAG);
            res->height--;
            if (nullEval >= beta)
            {
                makeNullMove();
#ifdef DEBUG
                count_NMP(1, depth, beta - evaluate());
#endif
                return beta;
            }
#ifdef DEBUG
            count_NMP(0, depth, beta - evaluate());
#endif
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

    // order most promising moves first
    orderMoves(movelist, size, res->height);

    orderFirst = 0;

    // clear killer moves for this ply
    killer_move(res->height + 1, 0) = 0;
    killer_move(res->height + 1, 1) = 0;

    int hasLegalMoves = 0;

    int mIdx = -1;

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
        // have at least a move before time runs out by immediately selecting the first legal move
        // when searching this position for the first time.
        if (!hasLegalMoves && isRoot && depth == 1)
        {
            res->bestMove = m;
        }
        hasLegalMoves = 1;

        // keep track of move index to avoid factoring in illegal moves as part of the list
        mIdx++;

#ifdef DEBUG
        count_move(m);
#endif

        res->height++;
        int eval = 0;
        // LMR is done for remaining moves
        if (!isPV && !is_move_capt(m) && mIdx >= 3 && depth >= 2 && !isInCheck && !isAttackingKing())
        {
            // From https://www.chessprogramming.org/Late_Move_Reductions
            int reduce = (int)(0.8 + log(depth) * log(mIdx) / 2.4);

            int newDepth = depth - 1 - reduce;
            if (newDepth < 0)
            {
                newDepth = 0;
            }

            // search with a null window (PVS)
            eval = -think(newDepth, -alpha - 1, -alpha, res, flags);
            // must search again
            if (eval > alpha)
            {
                eval = -think(depth - 1, -beta, -alpha, res, flags);
            }
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha, res, flags);
        }
        res->height--;

        unmakeMove(m);

        // make sure we are still allowed to think.
        if (res->stopThinking)
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
                writeToTranspositionTable(depth, beta, m, TT_UPPER, res->height);
            }
#endif            
            // for moves that do not capture...
            if (!is_move_capt(m))
            {
                // store killer move
                addKillerMove(m, res->height);

                // update history moves
                int bonus = 600 * depth * depth;

                updateHistory(m, bonus);

                // all previously searched quiet moves receive a negative score, as they did not
                // provide the desired cut-off. (history maluses)
                // this gives unpromising moves a negative score
                int malusBonus = -bonus;
                for (int j = 0; j < i; j++)
                {
                    Move mj = movelist[j];
                    if (!is_move_capt(mj))
                    {
                        updateHistory(mj, malusBonus);
                    }
                }
            }
#ifdef DEBUG
            count_nodeType(TT_UPPER);
            count_betaCutoff(mIdx, m);
#endif
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
                res->bestMove = m;
            }
        }
    }

    // if there are no legal moves in this position, it must be a stalemate.
    if (!hasLegalMoves)
    {
        return 0;
    }

#ifdef DEBUG
    count_nodeType(nodeType);
#endif

    // save this entry in the transposition table
#ifdef USE_TRANSPOSITION_TABLE
    if (!isNullMovePruning)
    {
        writeToTranspositionTable(depth, alpha, bestMove, nodeType, res->height);
    }
#endif

    // return best evaluation
    return alpha;
}

int thinkCaptures(int alpha, int beta, SearchResults* res, int accessTT)
{
#ifdef DEBUG
    count_nodeVisited(1);
#endif
    // avoids counting leaf nodes (as they have already been counted by the parent node)
    res->nodesVisited += !accessTT;
    determineThinkAllowance(res);

    // not capturing might be better
    int eval = evaluate();
    if (eval >= beta)
    {
#ifdef USE_TRANSPOSITION_TABLE
        if (accessTT)
        {
            writeToTranspositionTable(0, beta, 0, TT_UPPER, res->height);
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
    orderCapts(movelist, size);

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

        res->height++;
        int eval = -thinkCaptures(-beta, -alpha, res, 0);
        res->height--;

        unmakeMove(m);

        if (res->stopThinking)
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
                writeToTranspositionTable(0, beta, 0, TT_UPPER, res->height);
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
        writeToTranspositionTable(0, alpha, 0, nodeType, res->height);
    }
#endif

    // return best evaluation
    return alpha;
}
