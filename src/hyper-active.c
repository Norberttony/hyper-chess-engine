
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// move generation
#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"
#include "move.h"
#include "perft.h"
#include "test-suite.h"

// algorithm
#include "evaluate.h"
#include "transposition-table.h"

int thinkingTime = -1;
clock_t thinkStart = -1; // to-do: worried about the precision of clock_t
Move orderFirst = 0; // the move to order first
int maxDepth = 0;

int nodesVisited = 0;

// thinks in the allocated time and returns the best move it could find
Move thinkFor(int time);

// determines if engine is still allowed to think or not
int getThinkAllowance();

// gets the best move at the given depth
Move getBestMove(int depth);

// performs minmax search with alpha-beta pruning
int think(int depth, int alpha, int beta);

// performs minmax search with alpha-beta pruning for capturing moves only.
// depth only serves as a counter (to value longest/shortest sequences of mate)
int thinkCaptures(int alpha, int beta); // depth serves as a counter for finding fastest mate

// returns a positive value if the second move is greater (in ordering)
int compareMoves(const void*, const void*);

// prints out the principal variation from the current position
void printPrincipalVariation(int depth);

int main(void)
{
    // Initialization!
    generateZobristHashes();
    populateKingMoves();
    populateRanksAndFiles(); // in order to use genDeathSquares (used by populateDeathSquares)
    populateDeathSquares();
    populateSpringerLeaps();
    populateSpringerCaptures();
    populateRetractorCaptures();
    initMagicBitboards(0); // rook magic bitboards
    initMagicBitboards(1); // bishop magic bitboards

    runTestSuite();

    return 0;

    loadFEN(StartingFEN);
    getBestMove(8);

    return 0;

    srand(time(NULL));

    // prompt user for FEN
    puts("Paste in the FEN:");
    fflush(stdout);
    
    char chosenFEN[1000];
    fgets(chosenFEN, 1000, stdin);

    // load user's choice, or if that fails, the starting FEN
    if (!loadFEN(chosenFEN))
    {
        puts("Loading starting FEN instead...");
        loadFEN(StartingFEN);
    }

    puts("Ready to play!");

    // prompt user for which side they will play
    puts("Which side do you want to play? (w/b)");
    fflush(stdout);

    char sideInput = 0;
    do
    {
        sideInput = getchar();
        scanf("%*c"); // ignore newline character

        if (sideInput != 'w' && sideInput != 'b')
        {
            puts("That's not a valid side, choose either white (w) or black (b)");
        }
    } while (sideInput != 'w' && sideInput != 'b');

    int mySide = sideInput == 'w' ? black : white;

    prettyPrintBoard();

    // time to play the game!
    int gameOver = 0;
    while (!gameOver)
    {
        puts("");
        // yeah, a bit of a bad way to check for game-ending situations. oh well...
        Move movelist[MAX_MOVES];
        int size = generateMoves((Move*)movelist);
        // consider only legal moves
        int legalMoves = 0;
        for (int i = 0; i < size; i++)
        {
            legalMoves += isMoveLegal(movelist[i]);
        }
        if (legalMoves == 0)
        {
            if (isCheckmate())
            {
                if (toPlay == white)
                {
                    puts("result 0-1");
                }
                else
                {
                    puts("result 1-0");
                }
            }
            else
            {
                puts("result 1/2-1/2");
            }
            fflush(stdout);
            free(movelist);
            gameOver = 1;
            break;
        }
        free(movelist);

        // time to consider which moves are played
        if (mySide == toPlay)
        {
            puts("Thinking . . .");

            clock_t start = clock();
            Move best = getBestMove(5);
            clock_t end = clock();

            printf("Thought for %f seconds.\n", (float)(end - start) / CLOCKS_PER_SEC);

            printf("makemove %s%s\n", squareNames[(best & move_fromMask) >> 3], squareNames[(best & move_toMask) >> 9]);
            fflush(stdout);
            makeMove(best);
            prettyPrintBoard();
        }
        else
        {
            //puts("Your turn! Type in a move:");
            char move[10]; // yes, risks buffer overflow, I know...
            scanf("%s", &move);

            // get coordinates of from square and to square
            int fromF = move[0] - 'a';
            int fromR = 7 - (move[1] - '1');

            int toF = move[2] - 'a';
            int toR = 7 - (move[3] - '1');

            int from = fromR * 8 + fromF;
            int to = toR * 8 + toF;

            // try to play the move...
            if (!chooseMove(from, to))
            {
                puts("That's not a legal move");
            }
            else
            {
                prettyPrintBoard();
            }
        }
    }

    puts("Good game!");

    return 0;
}

Move getBestMove(int depth)
{
    nodesVisited++;

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist);

    if (size == 0)
    {
        return 0; // no moves!
    }

    // determine most promising moves
    qsort(movelist, size, sizeof(Move), compareMoves);

    int alpha = INT_MIN + 1;
    int beta = INT_MAX - 1;
    Move bestMove = orderFirst;
    for (int i = 0; i < size; i++)
    {
        Move m = movelist[i];
        if (!isMoveLegal(m))
        {
            continue;
        }

        makeMove(m);

        // try to maximize value
        int eval;

        // can't really see stalemates yet
        if (isCheckmate())
        {
            eval = (INT_MAX - 40) + depth;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // make sure we are still allowed to think.
        // if not, throw in the best move we've got!
        // not using break because then it will throw in a weird evaluation.
        if (!getThinkAllowance())
        {
            return bestMove;
        }

        if (eval > alpha)
        {
            alpha = eval;
            bestMove = m;
        }
    }

    printf("depth %d: evaluating the position as %d\n", depth, alpha);

    writeToTranspositionTable(zobristHash, depth, alpha, bestMove, TT_EXACT);

    return bestMove;
}

int think(int depth, int alpha, int beta)
{
    nodesVisited++;

    // used to test whether or not this node's evaluation broke out of the window (increased alpha)
    int originalAlpha = alpha;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
    struct TranspositionEntry savedEval = transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES];
    int isSavedEntryValid = savedEval.zobristHash == zobristHash && savedEval.depth >= depth;
    if (isSavedEntryValid)
    {
        depthHits[depth]++;
        TT_hits++;
        if (savedEval.nodeType == TT_EXACT)
        {
            return savedEval.eval;
        }
        else if (savedEval.nodeType == TT_LOWER && alpha < savedEval.eval)
        {
            alpha = savedEval.eval;
        }
        else if (savedEval.nodeType == TT_UPPER && beta > savedEval.eval)
        {
            beta = savedEval.eval;
        }

        if (alpha >= beta)
        {
            return savedEval.eval;
        }
    }
    else
    {
        TT_misses++;
        TT_misses_type1 += savedEval.zobristHash != 0 && savedEval.zobristHash != zobristHash;
    }
    // order based on saved entry
    orderFirst = savedEval.bestMove;

    if (depth == 0)
    {
        int eval = thinkCaptures(alpha, beta);
        if (transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES].depth == 0)
        {
            writeToTranspositionTable(zobristHash, 0, eval, 0, TT_EXACT);
        }
        else
        {
            TT_nowrites++;
        }
        return eval;
    }

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist);

    if (size == 0)
    {
        TT_nowrites++;
        return 0; // no moves! don't try sorting
    }

    // order most promising moves first
    qsort(movelist, size, sizeof(Move), compareMoves);

    int hasLegalMoves = 0;

    int isCutOffNode = 0;
    Move bestMove = movelist[0];
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
        int repeats = 0;
        for (int j = 0; j < REPEAT_TABLE_ENTRIES; j++)
        {
            repeats += repeatTable[j] == zobristHash;
        }

        if (isCheckmate())
        {
            // will never calculate mate in 40. right?
            eval = (INT_MAX - 40) + depth;
        }
        else if (repeats == 3)
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
            break;
        }

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            isCutOffNode = 1;
            alpha = beta;
            // store the move that caused the cut off (to possibly use it as a killer heuristic)
            bestMove = m;
            break;
        }

        if (eval > alpha)
        {
            alpha = eval;
            bestMove = m;
        }
    }

    // if there are no legal moves in this position, it must be a stalemate.
    if (!hasLegalMoves)
    {
        TT_nowrites++;
        return 0;
    }

    // save this entry in the transposition table (replace by depth)
    struct TranspositionEntry tt_entry = transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES];
    if (tt_entry.depth < depth)
    {
        if (isCutOffNode)
        {
            if (tt_entry.nodeType != TT_EXACT)
            {
                writeToTranspositionTable(zobristHash, depth, beta, bestMove, TT_LOWER);
            }
            else
            {
                TT_nowrites++;
            }
        }
        // this node broke out of alpha, shattering the original window. its children were
        // all calculated correctly, resulting in the exact score for this node.
        else if (alpha > originalAlpha)
        {
            writeToTranspositionTable(zobristHash, depth, alpha, bestMove, TT_EXACT);
        }
        // or maybe it didn't. in that case, this node never exceeded alpha.
        else if (alpha <= originalAlpha && tt_entry.nodeType != TT_EXACT)
        {
            writeToTranspositionTable(zobristHash, depth, alpha, bestMove, TT_UPPER);
        }
        else
        {
            TT_nowrites++;
        }
    }
    else
    {
        TT_nowrites++;
    }

    // return best evaluation
    return alpha;
}

int thinkCaptures(int alpha, int beta)
{
    int eval = evaluate();
    // not capturing might be better
    if (eval >= beta)
    {
        return beta;
    }
    if (eval > alpha)
    {
        alpha = eval;
    }

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*) movelist);

    if (size == 0)
    {
        free(movelist);
        return 0; // no moves!
    }

    // determine most promising moves
    qsort(movelist, size, sizeof(Move), compareMoves);

    for (int i = 0; i < size; i++)
    {
        Move m = movelist[i];
        if (!(m & move_captMask))
        {
            break; // done deciding on captures (which should all be first)
        }
        makeMove(m);

        if (isAttackingKing())
        {
            unmakeMove(m);
            continue;
        }

        int eval = -thinkCaptures(-beta, -alpha);

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
            // make this move slightly worse from the last one so that it is not chosen as equal
            // (as a candidate move) to the other moves in the branch above...
            return beta;
        }

        if (eval > alpha)
        {
            alpha = eval;
        }
    }

    // return best evaluation
    return alpha;
}

// orders captures first, and also orders the last best move first.
int compareMoves(const void *a, const void *b)
{
    // add some slight variety to the moves.
    // to-do: still problematic, as chameleon captures have less weight
    Move m1 = *((Move*)a);
    Move m2 = *((Move*)b);

    // order the orderFirst move the highest
    int val1 = (m2 == orderFirst) * move_captMask + (int)(m2 & move_captMask);
    int val2 = (m1 == orderFirst) * move_captMask + (int)(m1 & move_captMask);

    return val1 - val2;
}

Move thinkFor(int time)
{
    // thinking is back on schedule
    thinkStart = clock();
    thinkingTime = time;
    orderFirst = 0;

    U64 myHash = zobristHash;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
    Move bestMove = 0;
    while (getThinkAllowance())
    {
        TT_hits = 0;
        TT_misses = 0;
        TT_misses_type1 = 0;
        TT_entries_filled = 0;
        TT_entries_overwritten = 0;
        TT_nowrites = 0;

        nodesVisited = 0;

        for (int i = 0; i < depth + 1; i++)
        {
            depthHits[i] = 0;
        }

        printf("Searching at depth %d\n", depth + 1);
        printf("Order first: ");
        prettyPrintMove(orderFirst);

        maxDepth = depth + 1;
        Move candidate = getBestMove(++depth);

        puts("DEPTH HITS");
        for (int i = 0; i < maxDepth; i++)
        {
            printf("depth %d hits: %d\n", i, depthHits[i]);
        }
        puts("");

        // since this is the best move at this depth, it should cause massive cut offs at the next
        // level. A bit of a history heuristic :)
        orderFirst = candidate;

        bestMove = candidate;
        printf("(%+d) ", transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES].eval);
        printPrincipalVariation(depth);
        printf("\n");
        printf("Visited %d nodes\n", nodesVisited);
        printf("\n");

        puts("TT RESULTS");
        printf("%d hits\n", TT_hits);
        printf("%d misses\n", TT_misses);
        printf("%d type 1 errors\n", TT_misses_type1);
        printf("%d entries filled\n", TT_entries_filled);
        printf("%d entries overwritten\n", TT_entries_overwritten);
        printf("%d no writes\n\n", TT_nowrites);
    }

    if (myHash != zobristHash)
    {
        puts("PANIC! HASH ERROR!");
    }

    printf("Had to stop at depth %d\n", depth);

    // set back to always think
    thinkingTime = -1;

    return bestMove;
}

int getThinkAllowance()
{
    return thinkStart == -1 || (float)(clock() - thinkStart) / (float)((clock_t)1) < thinkingTime;
}

void printPrincipalVariation(int depth)
{
    // base case
    if (depth == 0)
    {
        return;
    }

    // get entry from transposition table
    struct TranspositionEntry entry = transpositionTable[zobristHash & TRANSPOSITION_TABLE_ENTRIES];

    // make sure it matches
    // for some reason, certain table entries are filled with a null move
    if (entry.zobristHash == zobristHash && entry.bestMove)
    {
        printMove(entry.bestMove);

        // update the zobrist hash, and keep printing the principal variation.
        makeMove(entry.bestMove);
        printPrincipalVariation(depth - 1);
        unmakeMove(entry.bestMove);
    }
}
