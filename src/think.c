
#include "think.h"

// how often the order first (from TT) performs
U64 orderFirstAttempts = 0;
U64 orderFirstSuccess = 0;

// search nodes and quiescent search nodes visited
U64 nodesVisited = 0;
#ifdef DEBUG
U64 qNodesVisited = 0;
#endif

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
U64 cumulativeNodesVisited = 0ULL;
U64 cumulativeQNodesVisited = 0ULL;
U64 cumulativeTime = 0ULL;
U64 cutoffFirst = 0ULL;
U64 cutoffSecond = 0ULL;
U64 cutoffThird = 0ULL;
U64 cutoffAvg = 0ULL;
U64 cutoffRemaining = 0ULL;
U64 cutoffRemainingAvg = 0ULL;
U64 cutoffFirstCapt = 0ULL;
U64 cutoffSecondCapt = 0ULL;
U64 captureCutoffs = 0ULL;
U64 noCaptureCutNodes = 0ULL;
U64 historyReliantCutNodes = 0ULL;
U64 goodKillers = 0ULL;
U64 badKillers = 0ULL;
int pieceTypeCutoffs[8] = { 0 };
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
    currBestMove = 0;

    U64 myHash = g_pos.zobristHash;

    U64 totalNodesVisited = 0ULL;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 1;
#ifdef DEBUG
    EVAL_DBG_PRINT = 1;
    printf("info string root-eval %d\n", (g_pos.toPlay == white ? 1 : -1) * evaluate());
    EVAL_DBG_PRINT = 0;
#endif
    while (!s->stopThinking && depth <= s->maxDepth)
    {
        nodesVisited = 0ULL;
#ifdef DEBUG
        qNodesVisited = 0ULL;
#endif

        int eval = think(depth, -MAX_SCORE, MAX_SCORE, IS_PV_FLAG);

        // handling checkmate (or stalemate) at the root
        if (depth > 0 && nodesVisited == 1ULL)
        {
            printf("info score ");
            printEval(eval);
            puts(" depth 0");
            break;
        }

        // since this is the best move at this depth, it will be ordered first (as it's likely
        // still the first move of the PV).
        orderFirst = currBestMove;

        totalNodesVisited += nodesVisited;
#ifdef DEBUG
        cumulativeNodesVisited += totalNodesVisited;
        cumulativeQNodesVisited += qNodesVisited;
#endif

        printf("info score ");
        printEval_TT();
        printf(" depth %d nodes %lld time %d pv ", depth, totalNodesVisited, getCurrentTime() - s->thinkStart);
        saveRepeatTable();
        if (s->stopThinking)
        {
            // at least print part of the current principal variation if prompted to stop thinking
            // if we changed moves at the root but interrupted thinking, the root hasn't had enough
            // time to put in the best move into the TT. So, we forcefully play it here and then
            // play out the PV. This might still give an inaccurate PV (because of missing TT
            // entries) but should be right a good amount of the time.
            printMove(currBestMove);
            makeMove(currBestMove);
            printPrincipalVariation(depth - 1, 2 * (depth - 1));
            unmakeMove(currBestMove);
        }
        else
        {
            printPrincipalVariation(depth, 2 * depth);
        }
        restoreRepeatTable();
        puts("");
        depth++;
    }

#ifdef DEBUG
    cumulativeTime += (U64)(getCurrentTime() - s->thinkStart);
#endif
    printf("bestmove %s%s\n", squareNames[get_from(currBestMove)], squareNames[get_to(currBestMove)]);

    if (myHash != g_pos.zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

#ifdef DEBUG
    U64 cutoffs = nodeOccurrence[TT_UPPER];
    puts("\nCumulative Stats");
    printf("Nodes visited: %lld\n", cumulativeNodesVisited);
    printf("QNodes visited: %lld\n", cumulativeQNodesVisited);
    printf("Nodes per second: %lf\n", (double)cumulativeNodesVisited * 1000 / cumulativeTime);
    printf("Total execution time (in ms): %lld\n", cumulativeTime);
    printf("# of cut-offs: %lld\n", cutoffs);
    printf("First move cut-off: %lf%%\n", 100 * (double)cutoffFirst / cutoffs);
    printf("Second move cut-off: %lf%%\n", 100 * (double)cutoffSecond / cutoffs);
    printf("Third move cut-off: %lf%%\n", 100 * (double)cutoffThird / cutoffs);
    printf("On average move cut off after: %lf moves\n", (double)cutoffAvg / cutoffs);
    printf("Cut-off in remaining moves (after hash, killers, captures): %lld (%lf%%)\n", cutoffRemaining, 100 * (double)cutoffRemaining / cutoffs);
    printf("Cut-off avg with remaining moves: %lf\n", (double)cutoffRemainingAvg / cutoffRemaining);
    printf("All nodes: %lld\n", nodeOccurrence[TT_LOWER]);
    printf("Cut nodes: %lld\n", nodeOccurrence[TT_UPPER]);
    printf("PV nodes: %lld\n", nodeOccurrence[TT_EXACT]);
    printf("# of cut-offs caused by a capture: %lld\n", captureCutoffs);
    printf("# of cut-offs caused by a capture ordered first: %lld\n", cutoffFirstCapt);
    printf("# of cut-offs caused by a capture ordered second: %lld\n", cutoffSecondCapt);
    printf("# of cut nodes without any captures: %lld\n", noCaptureCutNodes);
    printf("cut nodes that rely on history: %lld\n", historyReliantCutNodes);
    for (int i = straddler; i <= king; i++)
    {
        printf("%c - %d\n", pieceFEN[i], pieceTypeCutoffs[i]);
    }
    puts("");
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
    else if (!isRoot && (getNumberOfRepeats() >= 2 || g_pos.state->halfmove >= DRAW_MOVE_RULE))
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
    int moveScores[MAX_MOVES];
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
    orderMoves(movelist, size, g_searchParams.height, moveScores);

    // clear killer moves for this ply
    killer_move(g_searchParams.height + 1, 0) = 0;
    killer_move(g_searchParams.height + 1, 1) = 0;

    int hasLegalMoves = 0;

#ifdef DEBUG
    int mIdx = 0;
    int capts = 0;
    int k1Idx = -1;
    int k2Idx = -1;
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
        // have at least a move before time runs out by immediately selecting the first legal move
        // when searching this position for the first time.
        if (!hasLegalMoves && isRoot && depth == 1)
        {
            currBestMove = m;
        }
        hasLegalMoves = 1;

        g_searchParams.height++;
        int eval = -think(depth - 1, -beta, -alpha, flags);
        g_searchParams.height--;

        unmakeMove(m);

#ifdef DEBUG
        // keep track of move index to avoid factoring in illegal moves as part of the list
        // this is used for cutoff statistics later.
        mIdx++;
        capts += is_move_capt(m);
        Move* killers = &killer_move(g_searchParams.height, 0);
#endif

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
            if (!isNullMovePruning)
            {
                nodeOccurrence[TT_UPPER]++;
                
                cutoffFirst += mIdx == 1;
                cutoffSecond += mIdx == 2;
                cutoffThird += mIdx == 3;
                cutoffAvg += mIdx;

                int isKiller = killers[0] == m || killers[1] == m;

                // This code is for showing all of the poor move ordering choices with some data.
                if (mIdx > 1 && depth > 5)
                {
                    char fen[1024];
                    getFEN(fen, 1024);
                    printf("(depth %d) At position %s\n", depth, fen);
                    printf("Is first move from TT? %d\n", isFromTT);
                    puts("MOVE ORDERING:");
                    for (int j = 0; j < i; j++)
                    {
                        printf("%d. (%d) ", j + 1, moveScores[j]);
                        prettyPrintMove(movelist[j]);
                    }
                    printf("Whereas the good cut off move was at index %d (%d): ", mIdx, moveScores[mIdx - 1]);
                    prettyPrintMove(m);
                    printf("Killer moves are: ");
                    printMove(killers[0]);
                    printf(" ");
                    printMove(killers[1]);
                    puts("\n");
                }

                captureCutoffs += is_move_capt(m);
                cutoffFirstCapt += capts == 1 && is_move_capt(m);
                cutoffSecondCapt += capts == 2 && is_move_capt(m);
                noCaptureCutNodes += capts == 0;

                historyReliantCutNodes += !isFromTT && capts == 0 && !isKiller;

                goodKillers += isKiller;
                badKillers += (k1Idx > -1) + (k2Idx > -1);

                if (m != orderedFirst && !is_move_capt(m) && !isKiller)
                {
                    cutoffRemaining++;
                    cutoffRemainingAvg += mIdx;
                    pieceTypeCutoffs[get_type(m)]++;
                }
            }
#endif

            // for moves that do not capture...
            if (!is_move_capt(m))
            {
                // store killer move
                addKillerMove(m, g_searchParams.height);

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

            return beta;
        }

#ifdef DEBUG
        if (killers[0] == m)
        {
            k1Idx = mIdx;
        }
        if (killers[1] == m)
        {
            k2Idx = mIdx;
        }
#endif

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
    // avoids counting leaf nodes (as they have already been counted by the parent node)
    nodesVisited += !accessTT;

#ifdef DEBUG
    qNodesVisited++;
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
