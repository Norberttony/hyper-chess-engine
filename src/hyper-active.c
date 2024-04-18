
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

#define USE_TRANSPOSITION_TABLE

int thinkingTime = -1;
clock_t thinkStart = -1; // to-do: worried about the precision of clock_t
Move orderFirst = 0; // the move to order first
int maxDepth = 0;

// search nodes and quiescent search nodes visited
int nodesVisited = 0;
int qNodesVisited = 0;

// thinks in the allocated time and returns the best move it could find
Move thinkFor(int time);

// determines if engine is still allowed to think or not
int getThinkAllowance();

// gets the best move at the given depth
Move getBestMove(int depth);

// performs minmax search with alpha-beta pruning
int think(int depth, int alpha, int beta);

// performs minmax search with alpha-beta pruning for capturing moves only.
int thinkCaptures(int alpha, int beta, int accessTT);

// returns a positive value if the second move is greater (in ordering)
int compareMoves(const void*, const void*);

// prints out the principal variation from the current position
void printPrincipalVariation(int depth);

// returns 1 if a threefold has occurred, and 0 otherwise.
// of course, the flag can also be set when the position has occurred twice. This prevents the
// engine from playing on the verge of draw when it might be completely winning.
int getThreefoldFlag();

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

    //runTestSuite();

    /*
    loadFEN("1nb1kbnr/1pp2pp1/u7/1p6/PqBppP1p/PP6/4P1PP/RN1QKBNU w 10");

    // iterative deepening
    getBestMove(1);
    getBestMove(2);
    getBestMove(3);
    getBestMove(4);
    getBestMove(5);
    getBestMove(6);

    return 0;
    */

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
            gameOver = 1;
            break;
        }

        // time to consider which moves are played
        if (mySide == toPlay)
        {
            puts("Thinking . . .");

            clock_t start = clock();
            Move best = thinkFor(200);
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
    maxDepth = depth;
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
            return bestMove;
        }

        if (eval > alpha)
        {
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

int think(int depth, int alpha, int beta)
{
    nodesVisited++;

    int nodeType = TT_LOWER;

    // return the evaluation that might have been saved in the transposition table.
    // this shifts our window if the given evaluation is a lower/upper bound.
#ifdef USE_TRANSPOSITION_TABLE
    if (depth >= TT_MIN_DEPTH)
    {
        struct TranspositionEntry* savedEval = getTranspositionTableEntry(depth);
        if (savedEval)
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

            // order based on saved entry
            orderFirst = savedEval->bestMove;
        }
    }
#endif

    // perform quiescent search at leaf nodes
    if (depth == 0)
    {
        return thinkCaptures(alpha, beta, 1);
    }

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist);

    if (size == 0)
    {
        return 0; // no moves! don't try sorting
    }

    // order most promising moves first
    qsort(movelist, size, sizeof(Move), compareMoves);

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
            return beta;
        }

        if (eval > alpha)
        {
            nodeType = TT_EXACT;
            alpha = eval;
            bestMove = m;
        }
    }

    // if there are no legal moves in this position, it must be a stalemate.
    if (!hasLegalMoves)
    {
        return 0;
    }

    // save this entry in the transposition table
#ifdef USE_TRANSPOSITION_TABLE
    writeToTranspositionTable(depth, alpha, bestMove, nodeType);
#endif

    // return best evaluation
    return alpha;
}

int thinkCaptures(int alpha, int beta, int accessTT)
{
    qNodesVisited++;

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

    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist);

    if (size == 0)
    {
        return 0; // no moves!
    }

    // determine most promising moves
    qsort(movelist, size, sizeof(Move), compareMoves);

    int nodeType = TT_LOWER;

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

    int totalNodesVisited = 0;
    int totalQNodesVisited = 0;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
    Move bestMove = 0;
    while (getThinkAllowance())
    {
        nodesVisited = 0;
        qNodesVisited = 0;

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
        printf("\n");

        totalNodesVisited += nodesVisited;
    }

    printf("Total number of nodes visited: %d\n", totalNodesVisited);
    printf("Total number of quiescent nodes visited: %d\n", totalQNodesVisited);

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
    // a leaf node does not have a "best move" tied to it.
    if (depth == 0)
    {
        return;
    }

    // get entry from transposition table
    struct TranspositionEntry* entry = getTranspositionTableEntryPV(depth);

    if (entry && entry->bestMove)
    {
        printMove(entry->bestMove);

        // update the zobrist hash, and keep printing the principal variation.
        makeMove(entry->bestMove);
        printPrincipalVariation(depth - 1);
        unmakeMove(entry->bestMove);
    }
}

int getThreefoldFlag()
{
    int repeats = 0;
    for (int j = 0; j < REPEAT_TABLE_ENTRIES; j++)
    {
        repeats += repeatTable[j] == zobristHash;
    }

    return repeats >= 2;
}
