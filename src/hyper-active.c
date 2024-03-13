
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

int thinkingTime = -1;
clock_t thinkStart = -1; // to-do: worried about the precision of clock_t

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
        struct MoveList *movelist = generateMoves();
        // consider only legal moves
        int legalMoves = 0;
        for (int i = 0; i < movelist->size; i++)
        {
            legalMoves += isMoveLegal(movelist->list[i]);
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
    struct MoveList *movelist = generateMoves();

    if (movelist->size == 0)
    {
        free(movelist);
        return 0; // no moves!
    }

    // determine most promising moves
    qsort(movelist->list, movelist->size, sizeof(Move), compareMoves);

    int alpha = INT_MIN + 1;
    int beta = INT_MAX - 1;
    Move bestMove = 0;
    for (int i = 0; i < movelist->size; i++)
    {
        Move m = movelist->list[i];
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
            free(movelist);
            return bestMove;
        }

        if (eval > alpha)
        {
            alpha = eval;
            bestMove = m;
        }
    }

    free(movelist);
    printf("depth %d: evaluating the position as %d\n", depth, alpha);

    return bestMove;
}

int think(int depth, int alpha, int beta)
{
    if (depth == 0)
    {
        return thinkCaptures(alpha, beta);
    }

    struct MoveList *movelist = generateMoves();

    if (movelist->size == 0)
    {
        free(movelist);
        return 0; // no moves! don't try sorting
    }

    // order most promising moves first
    qsort(movelist->list, movelist->size, sizeof(Move), compareMoves);

    int hasLegalMoves = 0;

    for (int i = 0; i < movelist->size; i++)
    {
        Move m = movelist->list[i];
        if (!isMoveLegal(m))
        {
            continue;
        }
        hasLegalMoves = 1;

        makeMove(m);

        int eval;

        // check for three fold repetition
        int repeats = 0;
        for (int j = 0; j < REPEAT_TABLE_ENTRIES; j++)
        {
            repeats += repeatTable[j] == zobristHash;
        }

        if (repeats == 3)
        {
            eval = 0;
        }
        else if (isCheckmate())
        {
            // will never calculate mate in 40. right?
            eval = (INT_MAX - 40) + depth;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // make sure we are still allowed to think.
        if (!getThinkAllowance())
        {
            free(movelist);
            return beta;
        }

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            free(movelist);
            return beta;
        }

        if (eval > alpha)
        {
            alpha = eval;
        }
    }

    free(movelist);

    // if there are no legal moves in this position, it must be a stalemate.
    if (!hasLegalMoves)
    {
        return 0;
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

    struct MoveList *movelist = generateMoves();

    if (movelist->size == 0)
    {
        free(movelist);
        return 0; // no moves!
    }

    // determine most promising moves
    qsort(movelist->list, movelist->size, sizeof(Move), compareMoves);

    for (int i = 0; i < movelist->size; i++)
    {
        Move m = movelist->list[i];
        if (!(m & move_captMask))
        {
            break; // done deciding on captures (which should all be first)
        }
        if (!isMoveLegal(m))
        {
            continue;
        }

        makeMove(m);

        int eval = -thinkCaptures(-beta, -alpha);

        unmakeMove(m);

        if (!getThinkAllowance())
        {
            free(movelist);
            return beta;
        }

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            free(movelist);
            // make this move slightly worse from the last one so that it is not chosen as equal
            // (as a candidate move) to the other moves in the branch above...
            return beta;
        }

        if (eval > alpha)
        {
            alpha = eval;
        }
    }

    free(movelist);

    // return best evaluation
    return alpha;
}

// should generally prefer captures over non-captures
int compareMoves(const void *a, const void *b)
{
    // add some slight variety to the moves.
    // to-do: still problematic, as chameleon captures have less weight
    Move m1 = *((Move*)a) & move_captMask;
    Move m2 = *((Move*)b) & move_captMask;
    return m2 - m1;
}

Move thinkFor(int time)
{
    // thinking is back on schedule
    thinkStart = clock();
    thinkingTime = time;

    // perform "iterative deepening"
    // simply. search depth 1. then 2. then 3. until you're out of time.
    int depth = 0;
    Move bestMove = 0;
    while (getThinkAllowance())
    {
        printf("Searching at depth %d\n", depth + 1);
        Move candidate = getBestMove(++depth);

        if (getThinkAllowance())
        {
            bestMove = candidate;
        }
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
