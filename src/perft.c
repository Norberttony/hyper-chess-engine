
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

        int checkmate = isCheckmate();

        // create a counter for this move specifically
        struct MoveCounter temp = { 1, 0, 0, 0 };
        if (checkmate)
        {
            temp.checkmates++;
        }
        else
        {
            temp = countMoves(depth - 1);
        }

        // add stats from the temp counter
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
        printf("%s%s %d %d %d %d\n", squareNames[(move >> 3) & 0b111111], squareNames[(move >> 9) & 0b111111], temp.moves, temp.captureMoves, temp.pieceCaptures, temp.checkmates);

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

        int checkmate = isCheckmate();

        // create a counter for this move specifically
        struct MoveCounter temp = { 1, 0, 0, 0 };
        if (checkmate)
        {
            temp.checkmates++;
        }
        else
        {
            temp = countMoves(depth - 1);
        }

        // add stats from the temp counter
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
    //return 1; // pseudo-legal moves
    makeMove(m);

    int res = isPositionLegal();

    unmakeMove(m);

    return res;
}

int isPositionLegal()
{
    if (isAttackingKing()){
        return 0;
    }

    struct MoveList *moves = generateMoves();

    for (int i = 0; i < moves->size; i++)
    {
        makeMove(moves->list[i]);

        // just took the king. no good.
        if (position[toPlay + king] == 0ULL)
        {
            unmakeMove(moves->list[i]);
            free(moves);
            return 0;
        }

        unmakeMove(moves->list[i]);
    }

    free(moves);

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

int chooseMove(int startSq, int endSq)
{
    struct MoveList *movelist = generateMoves();

    for (int i = 0; i < movelist->size; i++)
    {
        Move m = movelist->list[i];

        if (!isMoveLegal(m))
        {
            continue;
        }

        if ((m & move_fromMask) >> 3 == startSq && (m & move_toMask) >> 9 == endSq)
        {
            makeMove(m);
            free(movelist);
            return m;
        }
    }

    puts("Could not find move!");
    free(movelist);
    return 0;
}

int isAttackingKing()
{
    // get squares that enemy immobilizer is not influencing
    U64 enemImm = position[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // make sure chameleons aren't nearby...
    // they aren't programmed to take the king directly
    U64 attacked = 0ULL;
    U64 chameleons = position[toPlay + chameleon] & notImmInfl;
    while (chameleons)
    {
        attacked |= kingMoves[pop_lsb(chameleons)];
        chameleons &= chameleons - 1;
    }

    // but also consider checks that come from death squares.
    // specifically, coordinator AND (king/chameleon) death squares.
    U64 totalBoard = position[white] | position[black];

    int targetKingSq = pop_lsb(position[notToPlay + king]);
    U64 kingBoard = position[toPlay + king];
    int realKingSq = pop_lsb(kingBoard);
    int kingSq = pop_lsb(kingBoard & notImmInfl);
    U64 king1Board = (kingBoard > 0) * (kingMoves[kingSq] & ~position[toPlay]);

    U64 coordPieceBoard = position[toPlay + coordinator];
    int coordSq = pop_lsb(coordPieceBoard);
    U64 coordBoard = ((coordPieceBoard & notImmInfl) > 0) * (
        rookAttacks[coordSq][((rookMasks[coordSq] & totalBoard) * rookMagics[coordSq]) >> (64 - rookMaskBitCount[coordSq])] |
        bishopAttacks[coordSq][((bishopMasks[coordSq] & totalBoard) * bishopMagics[coordSq]) >> (64 - bishopMaskBitCount[coordSq])]
    ) & ~totalBoard;

    U64 chamBoard = position[toPlay + chameleon];
    int cham1Sq = pop_lsb(chamBoard);
    U64 cham1Board = ((chamBoard & notImmInfl) > 0) * (kingMoves[cham1Sq] & ~totalBoard);
    int cham2Sq = pop_lsb(chamBoard - 1 & chamBoard);
    U64 cham2Board = (((chamBoard - 1 & chamBoard) & notImmInfl) > 0) * (kingMoves[cham2Sq] & ~totalBoard);
    
    int isCheck =
        // king stays coordinator moves
        sqFiles[realKingSq] == sqFiles[targetKingSq] && coordBoard & sqRanks[targetKingSq] ||
        sqRanks[realKingSq] == sqRanks[targetKingSq] && coordBoard & sqFiles[targetKingSq] ||
        // chameleon 1 stays coordinator moves
        sqFiles[cham1Sq] * (chamBoard > 0) == sqFiles[targetKingSq] && coordBoard & sqRanks[targetKingSq] ||
        sqRanks[cham1Sq] * (chamBoard > 0) == sqRanks[targetKingSq] && coordBoard & sqFiles[targetKingSq] ||
        // chameleon 2 stays coordinator moves
        sqFiles[cham2Sq] * ((chamBoard - 1 & chamBoard) > 0) == sqFiles[targetKingSq] && coordBoard & sqRanks[targetKingSq] ||
        sqRanks[cham2Sq] * ((chamBoard - 1 & chamBoard) > 0) == sqRanks[targetKingSq] && coordBoard & sqFiles[targetKingSq] ||
        // coordinator stays king moves
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[targetKingSq] && king1Board & sqRanks[targetKingSq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[targetKingSq] && king1Board & sqFiles[targetKingSq] ||
        // coordinator stays chameleon 1 moves
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[targetKingSq] && cham1Board & sqRanks[targetKingSq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[targetKingSq] && cham1Board & sqFiles[targetKingSq] ||
        // coordinator stays chameleon 2 moves
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[targetKingSq] && cham2Board & sqRanks[targetKingSq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[targetKingSq] && cham2Board & sqFiles[targetKingSq];

    return position[notToPlay + king] & attacked || isCheck;
}

int isCheckmate()
{
    // generate responses to the attack
    struct MoveList *movelist = generateMoves();
    
    for (int i = 0; i < movelist->size; i++)
    {
        // check if this is a legal response
        if (isMoveLegal(movelist->list[i]))
        {
            free(movelist);
            return 0;
        }
    }

    // make sure king is actually attacked
    int isAttacked = 1;

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // stalemate!!!
    if (isPositionLegal())
    {
        isAttacked = 0;
    }

    // toggle turn back
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // checkmate!!!
    free(movelist);
    return isAttacked;
}
