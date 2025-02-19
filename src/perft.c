
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
        printf("%s%s %d %d %d %d\n", squareNames[get_from(move)], squareNames[get_to(move)], temp.moves, temp.captureMoves, temp.pieceCaptures, temp.checkmates);

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
    int kingSq = pop_lsb(kingBoard);
    U64 king1Board = ((kingBoard & notImmInfl) > 0) * (kingMoves[kingSq] & ~position[toPlay]);

    // of course, the king can take an immobilized king.
    attacked |= (kingMoves[kingSq] & ~position[toPlay]) * ((kingBoard & notImmInfl) > 0);

    // coordinator AND chameleon(king)-coordinator checks!
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
        get_rook_attacks(coordSq, totalBoard) |
        get_bishop_attacks(coordSq, totalBoard)
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
    // check springer checks
    U64 springerBoard = position[stp + springer] & notImmInfl;
    U64 springer2Board = (springerBoard - 1) & springerBoard;

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
        U64 cham2Board = (chamBoard - 1) & chamBoard;

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

    if (coordPieceBoard & notImmInfl && (files[get_file(sq)] | ranks[get_rank(sq)]) & kingMovesBoard)
    {   
        coordBoard = (
            get_rook_attacks(coordSq, totalBoard) |
            get_bishop_attacks(coordSq, totalBoard)
        ) & ~totalBoard;
    }

    if (inclCham)
    {
        U64 chamBoard = position[stp + chameleon] & notImmInfl;
        U64 cham2Board = (chamBoard - 1) & chamBoard;

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
    U64 kingBoard = position[stp + king] & notImmInfl;
    U64 kingMovesBoard = (kingBoard > 0) * (kingMoves[pop_lsb(kingBoard)] & ~position[stp]);

    return (kingMovesBoard & (1ULL << sq)) > 0;
}

// assumes that the chameleon bits can never be in an illegal state (for instance, springer
// and straddler captures in one move)
int isMoveValid(Move m)
{
    int to = get_to(m);
    int from = get_from(m);
    int type = get_type(m);

    U64 toBoard = 1ULL << to;
    U64 fromBoard = 1ULL << from;

    U64 enemyImm = position[notToPlay + immobilizer];
    int enemyImmSq = pop_lsb(enemyImm);
    U64 enemyImmInfl = (enemyImm > 0) * kingMoves[enemyImmSq];

    if (pieceList[from] != type || !(position[toPlay] & fromBoard) || enemyImmInfl & fromBoard)
    {
        return 0;
    }

    // ensure valid captures and valid movement
    U64 totalBoard = position[white] | position[black];
    switch(type)
    {
        case straddler:
        {
            // check if can rook move to the toBoard
            U64 hasMove = toBoard & get_rook_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }

            // must look in each direction and ensure straddler is validly capturing
            // enemy piece exists one square after to, and a friendly straddler exists two squares after to.
            int upSq = to - 8;
            int ltSq = to - 1;
            int rtSq = to + 1;
            int dnSq = to + 8;
            int c1 = get_c1(m);
            int c2 = get_c2(m);
            int c3 = get_c3(m);
            int c4 = get_c4(m);
            U64 straddlers = position[toPlay + straddler];
            U64 chameleons = position[toPlay + chameleon];
            // it is possible that this was not a legal move, and that this straddler's upSq is beyond the s cope of the board.
            // this would cause undefined behavior.
            if (
                c1 && !(to > h7 && position[notToPlay + c1] & (1ULL << upSq) && (straddlers | ((c1 == straddler) * chameleons)) & (1ULL << (upSq - 8))) ||
                c2 && !(get_file(to) > 1 && position[notToPlay + c2] & (1ULL << ltSq) && (straddlers | ((c2 == straddler) * chameleons)) & (1ULL << (ltSq - 1))) ||
                c3 && !(get_file(to) < 6 && position[notToPlay + c3] & (1ULL << rtSq) && (straddlers | ((c3 == straddler) * chameleons)) & (1ULL << (rtSq + 1))) ||
                c4 && !(to < a2 && position[notToPlay + c4] & (1ULL << dnSq) && (straddlers | ((c4 == straddler) * chameleons)) & (1ULL << (dnSq + 8)))
            )
            {
                return 0;
            }
        }
        break;

        case retractor:
        {
            // valid move?
            U64 hasMove = toBoard & get_queen_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }
            
            // valid capture?
            int c1 = get_c1(m);
            U64 captBoard = retractorCaptures[from][to];
            if (c1 && !(position[notToPlay + c1] & captBoard))
            {
                return 0;
            }
        }
        break;

        case springer:
        {
            U64 attacks = get_queen_attacks(from, totalBoard);

            // valid capture?
            int c1 = get_c1(m);
            U64 captBoard = springerCaptures[from][to];
            if (c1 && !(position[notToPlay + c1] & captBoard & attacks))
            {
                return 0;
            }
            // valid move?
            if (!c1 && !(attacks & ~totalBoard & toBoard))
            {
                return 0;
            }
        }
        break;
        
        case coordinator:
        {
            // valid move?
            U64 hasMove = toBoard & get_queen_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }

            int kingSq = pop_lsb(position[toPlay + king]);

            // valid capture?
            int c1 = get_c1(m);
            int c2 = get_c2(m);
            U64 death1 = deathSquares[kingSq][to][0];
            U64 death2 = deathSquares[kingSq][to][1];
            if (
                c1 && !(position[notToPlay + c1] & death1) ||
                c2 && !(position[notToPlay + c2] & death2)
            )
            {
                return 0;
            }
        }
        break;
        
        case king:
        {
            // valid move?
            U64 hasMove = toBoard & kingMoves[from] & ~position[toPlay];
            if (!hasMove)
            {
                return 0;
            }

            int coordSq = pop_lsb(position[toPlay + coordinator]);

            // valid capture?
            int c1 = get_c1(m);
            int c2 = get_c2(m);
            int c3 = get_c3(m);

            int kb_c1 = get_kb_c1(m);
            int kb_c2 = get_kb_c2(m);
            int kb_c3 = get_kb_c3(m);
            int kb_c4 = get_kb_c4(m);

            U64 death1 = deathSquares[coordSq][to][0];
            U64 death2 = deathSquares[coordSq][to][1];

            U64 cham1 = position[toPlay + chameleon];
            int cham1Sq = pop_lsb(cham1);

            U64 death1b1 = deathSquares[to][cham1Sq][0];
            U64 death2b1 = deathSquares[to][cham1Sq][1];

            U64 cham2 = cham1 & (cham1 - 1);
            int cham2Sq = pop_lsb(cham2);

            U64 death1b2 = deathSquares[to][cham2Sq][0];
            U64 death2b2 = deathSquares[to][cham2Sq][1];

            U64 enemCoord = position[notToPlay + coordinator];

            if (
                c1 && !(pieceList[to] == c1) ||
                c2 && !(position[notToPlay + c2] & death1) ||
                c3 && !(position[notToPlay + c3] & death2) ||
                kb_c1 && cham1 && !(enemCoord & death1b1) ||
                kb_c2 && cham1 && !(enemCoord & death2b1) ||
                kb_c3 && cham2 && !(enemCoord & death1b2) ||
                kb_c4 && cham2 && !(enemCoord & death2b2)
            )
            {
                return 0;
            }
        }
        break;

        case chameleon:
        if (get_b_cn(m))
        {
            U64 attacks = get_queen_attacks(from, totalBoard);

            // valid capture?
            U64 captBoard = springerCaptures[from][to];
            if (!(position[notToPlay + springer] & captBoard & attacks))
            {
                return 0;
            }
            break;
        }

        // valid move?
        U64 hasMove = toBoard & get_queen_attacks(from, totalBoard) & ~totalBoard;
        if (!hasMove)
        {
            return 0;
        }

        if (get_b_cq(m))
        {
            // valid capture?
            U64 captBoard = retractorCaptures[from][to];
            if (!(position[notToPlay + retractor] & captBoard))
            {
                return 0;
            }
        }
        if (get_b_cp(m))
        {
            // check if can rook move to the toBoard
            U64 hasMove = toBoard & get_rook_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }
            U64 enemyStraddlers = position[notToPlay + straddler];
            U64 straddlers = position[toPlay + straddler] | position[toPlay + chameleon];
            int upSq = to - 8;
            int ltSq = to - 1;
            int rtSq = to + 1;
            int dnSq = to + 8;
            if (
                get_b_cu(m) && !(to > h7 && enemyStraddlers & (1ULL << upSq) && straddlers & (1ULL << (upSq - 8))) ||
                get_b_cl(m) && !(get_file(to) > 1 && enemyStraddlers & (1ULL << ltSq) && straddlers & (1ULL << (ltSq - 1))) ||
                get_b_cr(m) && !(get_file(to) < 6 && enemyStraddlers & (1ULL << rtSq) && straddlers & (1ULL << (rtSq + 1))) ||
                get_b_cd(m) && !(to < a2 && enemyStraddlers & (1ULL << dnSq) && straddlers & (1ULL << (dnSq + 8)))
            )
            {
                return 0;
            }
        }
        int d1 = get_b_cd1(m);
        int d2 = get_b_cd2(m);
        if (d1 | d2)
        {
            // valid move?
            U64 hasMove = toBoard & get_queen_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }

            int kingSq = pop_lsb(position[toPlay + king]);

            // valid capture?
            U64 death1 = deathSquares[kingSq][to][0];
            U64 death2 = deathSquares[kingSq][to][1];
            if (
                d1 && !(position[notToPlay + coordinator] & death1) ||
                d2 && !(position[notToPlay + coordinator] & death2)
            )
            {
                return 0;
            }
        }
        break;
        
        case immobilizer:
        {
            // valid move?
            U64 hasMove = toBoard & get_queen_attacks(from, totalBoard) & ~totalBoard;
            if (!hasMove)
            {
                return 0;
            }
        }
        break;
    }

    if (!isMoveLegal(m))
    {
        return 0;
    }
    else
    {
        return 1;
    }

    // return isMoveLegal(m);
}
