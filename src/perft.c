
#include "perft.h"

struct MoveCounter divide(int depth)
{
    if (depth == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    struct MoveList* moves = generateMoves();

    if (moves->size == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < moves->size; i++)
    {
        Move move = moves->list[i];

        if (!isMoveLegal(move))
        {
            continue;
        }

        makeMove(move);

        // add stats from counter
        temp = countMoves(depth - 1);
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

        if (move & move_captMask)
        {
            counter.captureMoves++;
        }
        
        int captureCount = countCaptures(move);
        counter.pieceCaptures += captureCount;

        // purpose of divide is to print the top level move and its move count
        printf("%s%s %d\n", squareNames[(move >> 3) & 0b111111], squareNames[(move >> 9) & 0b111111], temp.moves);

        // restore board state
        unmakeMove(move);
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

    if (moves->size == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    // create a counter and a temp for holding countMoves's return
    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < moves->size; i++)
    {
        Move move = moves->list[i];

        if (!isMoveLegal(move))
        {
            continue;
        }

        makeMove(move);

        // add stats from counter
        temp = countMoves(depth - 1);
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

        // check with capture mask
        if (move & move_captMask)
        {
            counter.captureMoves++;
        }
        
        counter.pieceCaptures += countCaptures(move);

        unmakeMove(move);
    }

    free(moves);
    
    return counter;
}

int isMoveLegal(Move m)
{
    return 1;
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

int countCaptures(Move move)
{
    switch(move & move_typeMask)
    {
        case straddler:
        case coordinator:
        case retractor:
        case springer:
            return
                ((move & move_c1Mask) > 0) +
                ((move & move_c2Mask) > 0) +
                ((move & move_c3Mask) > 0) +
                ((move & move_c4Mask) > 0);

        case chameleon:
            return
                ((move & move_cham_u_mask)  > 0) +
                ((move & move_cham_l_mask)  > 0) +
                ((move & move_cham_r_mask)  > 0) +
                ((move & move_cham_d_mask)  > 0) +
                ((move & move_cham_d1_mask) > 0) +
                ((move & move_cham_d2_mask) > 0) +
                ((move & move_cham_q_mask)  > 0) +
                ((move & move_cham_n_mask)  > 0);

        case king:
            return
                ((move & move_c1Mask) > 0) +
                ((move & move_c2Mask) > 0) +
                ((move & move_c3Mask) > 0) +
                ((move & move_kingc1mask) > 0) +
                ((move & move_kingc2mask) > 0) +
                ((move & move_kingc3mask) > 0) +
                ((move & move_kingc4mask) > 0);

        case immobilizer:
        default:
            return 0;
    }
}
