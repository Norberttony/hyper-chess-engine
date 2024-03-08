
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"
#include "move.h"
#include "perft.h"

const int pieceValues[] = { 0, 1, 3, 4, 8, 8, 6, 9999999 };

// greedy evaluation that counts material based on piece values
int evaluate();
Move getBestMove(int depth);
int think(int depth, int alpha, int beta);
int thinkCaptures(int depth, int alpha, int beta); // depth serves as a counter for finding fastest mate
int compareMoves(const void*, const void*);

int main(void)
{
    populateKingMoves();
    populateRanksAndFiles(); // in order to use genDeathSquares (used by populateDeathSquares)
    populateDeathSquares();
    populateSpringerLeaps();
    populateSpringerCaptures();
    populateRetractorCaptures();
    initMagicBitboards(0); // rook magic bitboards
    initMagicBitboards(1); // bishop magic bitboards

    // initial board set up
    loadFEN(StartingFEN);

    puts("Ready to play!");

    // prompt user for which side they will play
    puts("Which side do you want to play? (w/b)");

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
                puts("Checkmate!");
            }
            else
            {
                puts("Stalemate!");
            }
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

            prettyPrintMove(best);
            makeMove(best);
            prettyPrintBoard();
        }
        else
        {
            puts("Your turn! Type in a move:");
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

int evaluate()
{
    int evaluation = 0;
    for (int i = 1; i <= 6; i++)
    {
        U64 myBoard = position[toPlay + i];
        while (myBoard)
        {
            evaluation += pieceValues[i];
            myBoard &= myBoard - 1;
        }

        U64 enemyBoard = position[notToPlay + i];
        while (enemyBoard)
        {
            evaluation -= pieceValues[i];
            enemyBoard &= enemyBoard - 1;
        }
    }

    return evaluation;
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

    int alpha = INT_MIN + 10;
    int beta = INT_MAX - 10;
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
            eval = (INT32_MAX - 200) + depth;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }

        if (eval >= alpha)
        {
            alpha = eval;
            bestMove = m;
        }

        unmakeMove(m);
    }

    free(movelist);
    printf("Evaluating the position as %d\n", alpha);

    return bestMove;
}

int think(int depth, int alpha, int beta)
{
    if (depth == 0)
    {
        return thinkCaptures(depth - 1, alpha, beta);
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
        if (!isMoveLegal(m))
        {
            continue;
        }

        makeMove(m);

        int eval;

        if (isCheckmate())
        {
            // will never calculate mate in 200. right?
            eval = (INT32_MAX - 200) + depth;
        }
        else
        {
            eval = -think(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            free(movelist);
            // make this move slightly worse from the last one so that it is not chosen as equal
            // (as a candidate move) to the other moves in the branch above...
            return beta + 1;
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

int thinkCaptures(int depth, int alpha, int beta)
{
    int eval = evaluate();
    if (eval >= beta)
    {
        return beta + 1;
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

        int eval;

        if (isCheckmate())
        {
            // will never calculate mate in 200. right?
            eval = (INT32_MAX - 200) + depth;
        }
        else
        {
            eval = -thinkCaptures(depth - 1, -beta, -alpha);
        }
        unmakeMove(m);

        // alpha-beta pruning. if a move I play leads to a position where my opponent can play
        // something that's better than under a different move, there's no reason to consider
        // more moves from the opponent.
        if (eval >= beta)
        {
            free(movelist);
            // make this move slightly worse from the last one so that it is not chosen as equal
            // (as a candidate move) to the other moves in the branch above...
            return beta + 1;
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
    // but it's just move ordering, so it isn't the end of the world for now.
    // also! randomness added for some fun. otherwise AI often repeats moves.
    Move m1 = *((Move*)a) & move_captMask + rand() % (move_captMask >> 4);
    Move m2 = *((Move*)b) & move_captMask + rand() % (move_captMask >> 4);
    return m2 - m1;
}
