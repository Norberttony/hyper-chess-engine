
#include "perft.h"

struct MoveCounter divide(int depth)
{
    if (depth == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    Move moves[MAX_MOVES];
    int size = generateMoves((Move*)moves, 0);

    if (size == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < size; i++)
    {
        Move move = moves[i];

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

    return counter;
}

struct MoveCounter countMoves(int depth)
{
    if (depth == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    Move moves[MAX_MOVES];
    int size = generateMoves((Move*)moves, 0);

    if (size == 0)
    {
        struct MoveCounter counter = { 1, 0, 0, 0 };
        return counter;
    }

    // create a counter and a temp for holding countMoves's return
    struct MoveCounter counter = { 0, 0, 0, 0 };
    struct MoveCounter temp;

    for (int i = 0; i < size; i++)
    {
        Move move = moves[i];

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

    return counter;
}

int isMoveLegal(Move m)
{
    makeMove(m);

    int res = !isAttackingKing();

    unmakeMove(m);

    return res;
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
    Move moves[MAX_MOVES];
    int size = generateMoves(moves, 0);

    for (int i = 0; i < size; i++)
    {
        Move m = moves[i];

        if (!isMoveLegal(m))
        {
            continue;
        }

        if ((m & move_fromMask) >> 3 == startSq && (m & move_toMask) >> 9 == endSq)
        {
            makeMove(m);
            return m;
        }
    }

    puts("Could not find move!");
    return 0;
}

int chooseMoveBlind(int startSq, int endSq)
{
    Move moves[MAX_MOVES];
    int size = generateMoves(moves, 0);

    for (int i = 0; i < size; i++)
    {
        Move m = moves[i];

        if ((m & move_fromMask) >> 3 == startSq && (m & move_toMask) >> 9 == endSq)
        {
            makeMove(m);
            return m;
        }
    }

    puts("Could not find move!");
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

    U64 targetKing = position[notToPlay + king];
    int targetKingSq = pop_lsb(targetKing);
    U64 kingBoard = position[toPlay + king];
    int realKingSq = pop_lsb(kingBoard);
    int kingSq = pop_lsb(kingBoard);
    U64 king1Board = ((kingBoard & notImmInfl) > 0) * (kingMoves[kingSq] & ~position[toPlay]);

    // of course, the king can take an immobilized king.
    attacked |= (kingMoves[kingSq] & ~position[toPlay]) * ((kingBoard & notImmInfl) > 0);

    // coordinator AND chameleon(king)-coordinator checks!
    U64 coordPieceBoard = position[toPlay + coordinator];
    int coordSq = pop_lsb(coordPieceBoard);
    U64 coordBoard = ((coordPieceBoard & notImmInfl) > 0) * (
        get_rook_attacks(coordSq, totalBoard) |
        get_bishop_attacks(coordSq, totalBoard)
    ) & ~totalBoard;

    // board with (at most) two unimmobilized chameleons
    U64 chamBoard = position[toPlay + chameleon] & notImmInfl;
    // board with (at most) one unimmobilized chameleon
    U64 cham2Board = chamBoard & (chamBoard - 1);

    int cham1Sq = pop_lsb(chamBoard);
    int cham2Sq = pop_lsb(cham2Board);

    // kings and chameleons, whether immobilized or not.
    U64 kingAndChamBoard = kingBoard | position[toPlay + chameleon];

    // this works because a chameleon never captures by displacement...
    U64 kingAndChamKingMoves =
        (chamBoard > 0) * (kingMoves[cham1Sq] & ~totalBoard) |
        (cham2Board > 0) * (kingMoves[cham2Sq] & ~totalBoard) |
        king1Board;
    
    int isCheck =
        // king/chameleons stay, coordinator moves
        (sqFiles[targetKingSq] & kingAndChamBoard) > 0 && coordBoard & sqRanks[targetKingSq] ||
        (sqRanks[targetKingSq] & kingAndChamBoard) > 0 && coordBoard & sqFiles[targetKingSq] ||
        // coordinator stays, king/chameleons move
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[targetKingSq] && kingAndChamKingMoves & sqRanks[targetKingSq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[targetKingSq] && kingAndChamKingMoves & sqFiles[targetKingSq];

    // check springer checks
    U64 springerBoard = position[toPlay + springer] & notImmInfl;

    // springer 1
    int springer1Sq = pop_lsb(springerBoard);
    U64 springer1Board = (springerBoard > 0) * (
        get_rook_attacks(springer1Sq, totalBoard) |
        get_bishop_attacks(springer1Sq, totalBoard)
    ) & targetKing;

    // springer 2
    int springer2Sq = pop_lsb(springerBoard - 1 & springerBoard);
    U64 springer2Board = ((springerBoard - 1 & springerBoard) > 0) * (
        get_rook_attacks(springer2Sq, totalBoard) |
        get_bishop_attacks(springer2Sq, totalBoard)
    ) & targetKing;

    int isSpringerCheck =
        // springer 1
        springer1Board > 0 && springerLeaps[springer1Sq][targetKingSq] & ~totalBoard ||
        // springer 2
        springer2Board > 0 && springerLeaps[springer2Sq][targetKingSq] & ~totalBoard;

    // check retractor checks
    U64 retractorBoard = position[toPlay + retractor] & notImmInfl;
    int retractorSq = pop_lsb(retractorBoard);
    // ensures that retractor attacks king AND can land one square away
    int isRetractorCheck = retractorBoard && retractorCaptures[retractorSq][targetKingSq] & ~totalBoard;

    // check straddler checks
    // king is in check if straddler above, and another straddler can move below...
    U64 straddlerBoard = position[toPlay + straddler] & notImmInfl;

    int aboveSq = targetKingSq - 8;
    int belowSq = targetKingSq + 8;
    int leftSq  = targetKingSq - 1;
    int rightSq = targetKingSq + 1;

    int isAbove = aboveSq >= 0 && (position[toPlay + straddler] & (targetKing >> 8)); // true if straddler is above king
    int isBelow = belowSq < 64 && (position[toPlay + straddler] & (targetKing << 8));
    int isLeft  = targetKing & ~files[0] && (position[toPlay + straddler] & (targetKing >> 1));
    int isRight = targetKing & ~files[7] && (position[toPlay + straddler] & (targetKing << 1));

    int isStraddlerCheck =
        // check if straddler is ready to take the king because of a straddler above
        isAbove && (belowSq < 64) && !((targetKing << 8) & totalBoard) && (get_rook_attacks(belowSq, totalBoard) & straddlerBoard) ||
        // check if straddler is ready to take the king because of a straddler below
        isBelow && (aboveSq >= 0) && !((targetKing >> 8) & totalBoard) && (get_rook_attacks(aboveSq, totalBoard) & straddlerBoard) ||
        // check if straddler is ready to take the king because of a straddler to the left
        isLeft && targetKing & ~files[7] && !((targetKing << 1) & totalBoard) && (get_rook_attacks(rightSq, totalBoard) & straddlerBoard) ||
        // check if straddler is ready to take the king because of a straddler to the left
        isRight && targetKing & ~files[0] && !((targetKing >> 1) & totalBoard) && (get_rook_attacks(leftSq, totalBoard) & straddlerBoard);

    return position[notToPlay + king] & attacked || isCheck || isSpringerCheck || isRetractorCheck || isStraddlerCheck;
}

int isCheckmate()
{
    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // make sure king is actually attacked
    // if not, then it cannot be checkmate.
    int isAttacked = isAttackingKing();

    // toggle turn back
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    if (!isAttacked)
    {
        return 0;
    }

    // generate responses to the attack
    Move moves[MAX_MOVES];
    int size = generateMoves(moves, 0);
    
    for (int i = 0; i < size; i++)
    {
        // check if this is a legal response
        if (isMoveLegal(moves[i]))
        {
            return 0;
        }
    }

    // checkmate!!!
    return 1;
}
