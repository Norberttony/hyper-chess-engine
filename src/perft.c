
#include "perft.h"


void printMoveCounter(MoveCounter c)
{
    printf("%lld %lld %lld %lld\n", c.moves, c.captureMoves, c.pieceCaptures, c.checkmates);
}

MoveCounter divide(int depth, Move prevMove)
{
    if (depth == 0)
    {
        return (MoveCounter){
            1, 0, 0, 0
        };
    }

    Move moves[MAX_MOVES];
    int size = generateMoves((Move*)moves, 0);

    MoveCounter counter =
    {
        0, 0, 0, 0
    };

    for (int i = 0; i < size; i++)
    {
        Move move = moves[i];

        if (!isMoveLegal(move))
        {
            continue;
        }

        makeMove(move);

        MoveCounter temp = countMoves(depth - 1, move);

        // add stats from the temp counter
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

        // purpose of divide is to print the top level move and its move count
        const char* from = squareNames[get_from(move)];
        const char* to = squareNames[get_to(move)];
        printf("%s%s ", from, to);
        printMoveCounter(temp);

        // restore board state
        unmakeMove(move);
    }

    printf("\nTotal: ");
    printMoveCounter(counter);

    return counter;
}

MoveCounter countMoves(int depth, Move prevMove)
{
    if (depth == 0)
    {
        if (!prevMove)
        {
            return (MoveCounter){
                1, 0, 0, 0
            };
        }
        return (MoveCounter){
            1,
            is_move_capt(prevMove),
            countCaptures(prevMove),
            isCheckmate()
        };
    }

    Move moves[MAX_MOVES];
    int size = generateMoves((Move*)moves, 0);

    // create a counter and a temp for holding countMoves's return
    MoveCounter counter =
    {
        0, 0, 0, 0
    };

    for (int i = 0; i < size; i++)
    {
        Move move = moves[i];

        if (!isMoveLegal(move))
        {
            continue;
        }

        makeMove(move);

        MoveCounter temp = countMoves(depth - 1, move);

        // add stats from the temp counter
        counter.moves           += temp.moves;
        counter.captureMoves    += temp.captureMoves;
        counter.pieceCaptures   += temp.pieceCaptures;
        counter.checkmates      += temp.checkmates;

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
    U64* position = g_pos.boards;
    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;
    U64 targetKing = position[notToPlay + king];
    int targetKingSq = pop_lsb(targetKing);

    // get squares that enemy immobilizer is not influencing
    U64 enemImm = position[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // make sure chameleons and king aren't nearby...
    U64 manAttackers = (position[toPlay + king] | position[toPlay + chameleon]) & notImmInfl;
    if (manAttackers & kingMoves[targetKingSq])
    {
        return 1;
    }

    // but also consider checks that come from death squares.
    // specifically, coordinator AND (king/chameleon) death squares.
    U64 totalBoard = position[white] | position[black];
    U64 kingBoard = position[toPlay + king];
    int kingSq = pop_lsb(kingBoard);
    U64 king1Board = ((kingBoard & notImmInfl) > 0) * (kingMoves[kingSq] & ~position[toPlay]);

    // coordinator AND (chameleon/king)-coordinator checks!
    // board with (at most) two unimmobilized chameleons
    U64 chamBoard = position[toPlay + chameleon] & notImmInfl;
    // board with (at most) one unimmobilized chameleon
    U64 cham2Board = chamBoard & ((chamBoard - 1));

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
        get_queen_attacks(coordSq, totalBoard)
    ) & ~totalBoard;

    U64 targetKingFile = files[get_file(targetKingSq)];
    U64 targetKingRank = ranks[get_rank(targetKingSq)];

    U64 coordFile = files[get_file(coordSq)];
    U64 coordRank = ranks[get_rank(coordSq)];
    
    int isCheck =
        // king/chameleons stay, coordinator moves
        (targetKingFile & kingAndChamBoard) > 0 && coordBoard & targetKingRank ||
        (targetKingRank & kingAndChamBoard) > 0 && coordBoard & targetKingFile ||
        // coordinator stays, king/chameleons move
        coordFile * (coordPieceBoard > 0) == targetKingFile && kingAndChamKingMoves & targetKingRank ||
        coordRank * (coordPieceBoard > 0) == targetKingRank && kingAndChamKingMoves & targetKingFile;

    if (isCheck)
    {
        return 1;
    }

    if (isSquareControlledBySpringer(toPlay, targetKingSq, notImmInfl, totalBoard, 0))
    {
        return 1;
    }

    if (isSquareControlledByRetractor(toPlay, targetKingSq, notImmInfl, totalBoard, 0))
    {
        return 1;
    }

    if (isSquareControlledByStraddler(toPlay, targetKingSq, notImmInfl, totalBoard, 0))
    {
        return 1;
    }

    return 0;
}

int isCheckmate(void)
{
    // toggle turn
    g_pos.toPlay = !g_pos.toPlay * 8;
    g_pos.notToPlay = !g_pos.notToPlay * 8;

    // make sure king is actually attacked
    // if not, then it cannot be checkmate.
    int isAttacked = isAttackingKing();

    // toggle turn back
    g_pos.toPlay = !g_pos.toPlay * 8;
    g_pos.notToPlay = !g_pos.notToPlay * 8;

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
    U64 myImm = g_pos.boards[!stp * 8 + immobilizer];
    U64 notImmInfl = ~((myImm > 0) * kingMoves[pop_lsb(myImm)]);

    // looking for checking patterns often relies on the targetSq bit being turned on.
    U64 totalBoard = g_pos.boards[white] | g_pos.boards[black] | (1ULL << targetSq);

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
    U64 straddlerBoard = g_pos.boards[stp + straddler] | (inclCham * g_pos.boards[stp + chameleon]);
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
    U64 retractorBoard = g_pos.boards[stp + retractor] & notImmInfl;
    int retractorSq = pop_lsb(retractorBoard);

    if (inclCham)
    {
        U64 chamBoard = g_pos.boards[stp + chameleon] & notImmInfl;
        U64 cham2Board = (chamBoard - 1) & chamBoard;

        return
            chamBoard  && retractorCaptures[pop_lsb(chamBoard)][sq] & ~totalBoard ||
            cham2Board && retractorCaptures[pop_lsb(cham2Board)][sq] & ~totalBoard;
    }

    // ensures that retractor attacks king AND can land one square away
    return retractorBoard && retractorCaptures[retractorSq][sq] & ~totalBoard;
}

int isSquareControlledBySpringer(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    // all springers currently facing this square (potentially threatening capture)
    U64 springers = g_pos.boards[stp + springer] & notImmInfl;
    U64 chameleons = (g_pos.boards[stp + chameleon] & notImmInfl) * (inclCham != 0);
    U64 facingSpringers = get_queen_attacks(sq, totalBoard) & (springers | chameleons);

    while (facingSpringers)
    {
        int springerSq = pop_lsb(facingSpringers);
        if (springerLeaps[springerSq][sq] & ~totalBoard)
        {
            return 1;
        }
        facingSpringers &= facingSpringers - 1;
    }

    return 0;
}

int isSquareControlledByCoordinator(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham)
{
    U64 kingBoard = g_pos.boards[stp + king] & notImmInfl;
    U64 kingMovesBoard = (kingBoard > 0) * (kingMoves[pop_lsb(kingBoard)] & ~g_pos.boards[stp]);

    U64 coordPieceBoard = g_pos.boards[stp + coordinator];
    int coordSq = pop_lsb(coordPieceBoard);
    U64 coordBoard = 0ULL;

    if (coordPieceBoard & notImmInfl && (files[get_file(sq)] | ranks[get_rank(sq)]) & kingMovesBoard)
    {   
        coordBoard = (get_queen_attacks(coordSq, totalBoard)) & ~totalBoard;
    }

    if (inclCham)
    {
        U64 chamBoard = g_pos.boards[stp + chameleon] & notImmInfl;
        U64 cham2Board = (chamBoard - 1) & chamBoard;

        int cham1Sq = pop_lsb(chamBoard);
        int cham2Sq = pop_lsb(cham2Board);

        coordBoard |= (chamBoard > 0) * (get_queen_attacks(cham1Sq, totalBoard)) & ~totalBoard;

        coordBoard |= (cham2Board > 0) * (get_queen_attacks(cham2Sq, totalBoard)) & ~totalBoard;
    }

    U64 sqFile = files[get_file(sq)];
    U64 sqRank = ranks[get_rank(sq)];
    
    return
        // king stays, coordinator moves
        (sqFile & kingMovesBoard) > 0 && coordBoard & sqRank ||
        (sqRank & kingMovesBoard) > 0 && coordBoard & sqFile ||
        // coordinator stays, king moves
        files[get_file(coordSq)] * (coordPieceBoard > 0) == sqFile && kingMovesBoard & sqRank ||
        ranks[get_rank(coordSq)] * (coordPieceBoard > 0) == sqRank && kingMovesBoard & sqFile;
}

int isSquareControlledByKing(int stp, int sq, U64 notImmInfl, U64 totalBoard)
{
    U64 kingBoard = g_pos.boards[stp + king] & notImmInfl;
    U64 kingMovesBoard = (kingBoard > 0) * (kingMoves[pop_lsb(kingBoard)] & ~g_pos.boards[stp]);

    return (kingMovesBoard & (1ULL << sq)) > 0;
}
