
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

        if (is_move_capt(move))
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
        if (is_move_capt(move))
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
    switch(get_type(move))
    {
        case straddler:
        case coordinator:
        case retractor:
        case springer:
            return
                (get_c1(move) > 0) +
                (get_c2(move) > 0) +
                (get_c3(move) > 0) +
                (get_c4(move) > 0);

        case chameleon:
            return
                (get_b_cu(move)  > 0) +
                (get_b_cl(move)  > 0) +
                (get_b_cr(move)  > 0) +
                (get_b_cd(move)  > 0) +
                (get_b_cd1(move) > 0) +
                (get_b_cd2(move) > 0) +
                (get_b_cq(move)  > 0) +
                (get_b_cn(move)  > 0);

        case king:
            return
                (get_c1(move)    > 0) +
                (get_c2(move)    > 0) +
                (get_c3(move)    > 0) +
                (get_kb_c1(move) > 0) +
                (get_kb_c2(move) > 0) +
                (get_kb_c3(move) > 0) +
                (get_kb_c4(move) > 0);

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

        if (get_from(m) == startSq && get_to(m) == endSq)
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

        if (get_from(m) == startSq && get_to(m) == endSq)
        {
            makeMove(m);
            return m;
        }
    }

    puts("Could not find move!");
    return 0;
}

int isAttackingKing(void)
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

    U64 coordPieceBoard = position[toPlay + coordinator];
    int coordSq = pop_lsb(coordPieceBoard);
    U64 coordBoard = ((coordPieceBoard & notImmInfl) > 0) * (
        get_rook_attacks(coordSq, totalBoard) |
        get_bishop_attacks(coordSq, totalBoard)
    ) & ~totalBoard;
    
    int isCheck =
        // king/chameleons stay, coordinator moves
        (sqFiles[targetKingSq] & kingAndChamBoard) > 0 && coordBoard & sqRanks[targetKingSq] ||
        (sqRanks[targetKingSq] & kingAndChamBoard) > 0 && coordBoard & sqFiles[targetKingSq] ||
        // coordinator stays, king/chameleons move
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[targetKingSq] && kingAndChamKingMoves & sqRanks[targetKingSq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[targetKingSq] && kingAndChamKingMoves & sqFiles[targetKingSq];

    int isSpringerCheck = isSquareControlledBySpringer(toPlay, targetKingSq, notImmInfl, totalBoard, 0);

    int isRetractorCheck = isSquareControlledByRetractor(toPlay, targetKingSq, notImmInfl, totalBoard, 0);

    int isStraddlerCheck = isSquareControlledByStraddler(toPlay, targetKingSq, notImmInfl, totalBoard, 0);

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

int isSquareControlled(int stp, int targetSq, int pieceType)
{
    U64 myImm = position[!stp * 8 + immobilizer];
    U64 notImmInfl = ~((myImm > 0) * kingMoves[pop_lsb(myImm)]);

    // looking for checking patterns often relies on the targetSq bit being turned on.
    U64 totalBoard = position[white] | position[black] | (1ULL << targetSq);

    if (isSquareControlledByStraddler(stp, targetSq, notImmInfl, totalBoard, pieceType == straddler))
    {
        return straddler;
    }
    else if (isSquareControlledByRetractor(stp, targetSq, notImmInfl, totalBoard, pieceType == retractor))
    {
        return retractor;
    }
    else if (isSquareControlledBySpringer(stp, targetSq, notImmInfl, totalBoard, pieceType == springer))
    {
        return springer;
    }
    else if (isSquareControlledByCoordinator(stp, targetSq, notImmInfl, totalBoard, pieceType == coordinator))
    {
        return coordinator;
    }
    else if (isSquareControlledByKing(stp, targetSq, notImmInfl, totalBoard))
    {
        return king;
    }

    return 0;
}

int isSquareControlledByStraddler(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    U64 straddlerBoard = position[stp + straddler] | (inclCham * position[stp + chameleon]);
    U64 activeStraddlerBoard = straddlerBoard & notImmInfl;
    U64 sqBoard = 1ULL << sq;

    int aboveSq = sq - 8;
    int belowSq = sq + 8;
    int leftSq  = sq - 1;
    int rightSq = sq + 1;

    int isAbove = aboveSq >= 0 && (straddlerBoard & (sqBoard >> 8)); // true if straddler is above target
    int isBelow = belowSq < 64 && (straddlerBoard & (sqBoard << 8));
    int isLeft  = sqBoard & ~files[0] && (straddlerBoard & (sqBoard >> 1));
    int isRight = sqBoard & ~files[7] && (straddlerBoard & (sqBoard << 1));

    // need: straddler above, and empty square below where another straddler can move to. This
    // pattern is repeated for each of the directions.
    U64 isVerticalCheck = 0;
    if (isAbove && belowSq < 64 && !((sqBoard << 8) & totalBoard))
    {
        isVerticalCheck = get_rook_attacks(belowSq, totalBoard) & activeStraddlerBoard;
    }
    else if (isBelow && aboveSq >= 0 && !((sqBoard >> 8) & totalBoard))
    {
        isVerticalCheck = get_rook_attacks(aboveSq, totalBoard) & activeStraddlerBoard;
    }

    U64 isHorizontalCheck = 0;
    if (isLeft && sqBoard & ~files[7] && !((sqBoard << 1) & totalBoard))
    {
        isHorizontalCheck = get_rook_attacks(rightSq, totalBoard) & activeStraddlerBoard;
    }
    else if (isRight && sqBoard & ~files[0] && !((sqBoard >> 1) & totalBoard))
    {
        isHorizontalCheck = get_rook_attacks(leftSq, totalBoard) & activeStraddlerBoard;
    }

    return isVerticalCheck || isHorizontalCheck;
}

int isSquareControlledByRetractor(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    U64 retractorBoard = position[stp + retractor] & notImmInfl;
    int retractorSq = pop_lsb(retractorBoard);

    if (inclCham)
    {
        U64 chamBoard = position[stp + chameleon] & notImmInfl;
        U64 cham2Board = chamBoard - 1 & chamBoard;

        return
            chamBoard  && retractorCaptures[pop_lsb(chamBoard)][sq] & ~totalBoard ||
            cham2Board && retractorCaptures[pop_lsb(cham2Board)][sq] & ~totalBoard;
    }

    // ensures that retractor attacks king AND can land one square away
    return retractorBoard && retractorCaptures[retractorSq][sq] & ~totalBoard;
}

int isSquareControlledBySpringer(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    // check springer checks
    U64 springerBoard = position[stp + springer] & notImmInfl;
    U64 springer2Board = springerBoard - 1 & springerBoard;

    // springer 1
    int springer1Sq = pop_lsb(springerBoard);
    U64 springer1Attack = 0ULL;
    if (springerBoard && springerLeaps[springer1Sq][sq] & ~totalBoard)
    {
        springer1Attack = (
            get_rook_attacks(springer1Sq, totalBoard) |
            get_bishop_attacks(springer1Sq, totalBoard)
        ) & (1ULL << sq);
    }

    // springer 2
    int springer2Sq = pop_lsb(springer2Board);
    U64 springer2Attack = 0ULL;
    if (springer2Board && springerLeaps[springer2Sq][sq] & ~totalBoard)
    {
        // springer 2
        springer2Attack = (
            get_rook_attacks(springer2Sq, totalBoard) |
            get_bishop_attacks(springer2Sq, totalBoard)
        ) & (1ULL << sq);
    }

    int isSpringerCheck = springer1Attack || springer2Attack;

    if (inclCham && !isSpringerCheck)
    {
        U64 chamBoard = position[stp + chameleon] & notImmInfl;
        U64 cham2Board = chamBoard - 1 & chamBoard;

        // chameleon 1
        int cham1Sq = pop_lsb(chamBoard);
        U64 cham1Attack = (chamBoard > 0) * (
            get_rook_attacks(cham1Sq, totalBoard) |
            get_bishop_attacks(cham1Sq, totalBoard)
        ) & (1ULL << sq);

        // chameleon 2
        int cham2Sq = pop_lsb(cham2Board);
        U64 cham2Attack = (cham2Board > 0) * (
            get_rook_attacks(cham2Sq, totalBoard) |
            get_bishop_attacks(cham2Sq, totalBoard)
        ) & (1ULL << sq);

        return
            // cham 1
            cham1Attack && springerLeaps[cham1Sq][sq] & ~totalBoard ||
            // cham 2
            cham2Attack && springerLeaps[cham2Sq][sq] & ~totalBoard;
    }

    return isSpringerCheck;
}

int isSquareControlledByCoordinator(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    U64 kingBoard = position[stp + king] & notImmInfl;
    U64 kingMovesBoard = (kingBoard > 0) * (kingMoves[pop_lsb(kingBoard)] & ~position[stp]);

    U64 coordPieceBoard = position[stp + coordinator];
    int coordSq = pop_lsb(coordPieceBoard);
    U64 coordBoard = 0ULL;

    if (coordPieceBoard & notImmInfl && (sqFiles[sq] | sqRanks[sq]) & kingMovesBoard)
    {   
        coordBoard = (
            get_rook_attacks(coordSq, totalBoard) |
            get_bishop_attacks(coordSq, totalBoard)
        ) & ~totalBoard;
    }

    if (inclCham)
    {
        U64 chamBoard = position[stp + chameleon] & notImmInfl;
        U64 cham2Board = chamBoard - 1 & chamBoard;

        int cham1Sq = pop_lsb(chamBoard);
        int cham2Sq = pop_lsb(cham2Board);

        coordBoard |= (chamBoard > 0) * (
            get_rook_attacks(cham1Sq, totalBoard) |
            get_bishop_attacks(cham1Sq, totalBoard)
        ) & ~totalBoard;

        coordBoard |= (cham2Board > 0) * (
            get_rook_attacks(cham2Sq, totalBoard) |
            get_bishop_attacks(cham2Sq, totalBoard)
        ) & ~totalBoard;
    }
    
    return
        // king stays, coordinator moves
        (sqFiles[sq] & kingMovesBoard) > 0 && coordBoard & sqRanks[sq] ||
        (sqRanks[sq] & kingMovesBoard) > 0 && coordBoard & sqFiles[sq] ||
        // coordinator stays, king moves
        sqFiles[coordSq] * (coordPieceBoard > 0) == sqFiles[sq] && kingMovesBoard & sqRanks[sq] ||
        sqRanks[coordSq] * (coordPieceBoard > 0) == sqRanks[sq] && kingMovesBoard & sqFiles[sq];
}

int isSquareControlledByKing(int stp, int sq, U64 notImmInfl, U64 totalBoard)
{
    U64 kingBoard = position[stp + king] & notImmInfl;
    U64 kingMovesBoard = (kingBoard > 0) * (kingMoves[pop_lsb(kingBoard)] & ~position[stp]);

    return (kingMovesBoard & (1ULL << sq)) > 0;
}
