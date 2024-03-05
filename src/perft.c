
#include "perft.h"

struct MoveCounter divide(int depth)
{
    if (depth == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    struct MoveList* moves = generateMoves();

    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < moves->size; i++)
    {
        if (!isMoveLegal(moves->list[i]))
        {
            continue;
        }

        makeMove(moves->list[i]);

        // add stats from counter
        temp = countMoves(depth - 1);
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

        if (moves->list[i] & move_captMask)
        {
            counter.captureMoves++;
        }
        
        // to-do: this will be different from some pieces in the future.
        counter.pieceCaptures += 
            ((moves->list[i] & move_c1Mask) > 0) +
            ((moves->list[i] & move_c2Mask) > 0) +
            ((moves->list[i] & move_c3Mask) > 0) +
            ((moves->list[i] & move_c4Mask) > 0);

        // purpose of divide is to print the top level move and its move count
        printf("%s%s %d\n", squareNames[(moves->list[i] >> 3) & 0b111111], squareNames[(moves->list[i] >> 9) & 0b111111], temp.moves);

        // restore board state
        unmakeMove(moves->list[i]);
    }

    free(moves);
    
    return counter;
}

struct MoveCounter countMoves(int depth)
{
    if (depth == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    struct MoveList* moves = generateMoves();

    // create a counter and a temp for holding countMoves's return
    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < moves->size; i++)
    {
        if (!isMoveLegal(moves->list[i]))
        {
            continue;
        }
        
        makeMove(moves->list[i]);

        // add stats from counter
        temp = countMoves(depth - 1);
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

        // check with capture mask
        if (moves->list[i] & move_captMask)
        {
            counter.captureMoves++;
        }
        
        // to-do: this will be different from some pieces in the future.
        counter.pieceCaptures += 
            ((moves->list[i] & move_c1Mask) > 0) +
            ((moves->list[i] & move_c2Mask) > 0) +
            ((moves->list[i] & move_c3Mask) > 0) +
            ((moves->list[i] & move_c4Mask) > 0);

        unmakeMove(moves->list[i]);
    }

    free(moves);
    
    return counter;
}

int isMoveLegal(Move m)
{
    makeMove(m);

    struct MoveList *moves = generateMoves();

    for (int i = 0; i < moves->size; i++)
    {
        makeMove(moves->list[i]);

        // just took the king. no good.
        if (position[toPlay + king] == 0ULL)
        {
            unmakeMove(moves->list[i]);
            unmakeMove(m);
            free(moves);
            return 0;
        }

        unmakeMove(moves->list[i]);
    }

    free(moves);

    unmakeMove(m);
    return 1;
}
