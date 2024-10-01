
#include "move.h"

const char* squareNames[] =
{
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

const int move_typeMask     = 0b0000000000000000000000000111; // piece performing move
const int move_fromMask     = 0b0000000000000000000111111000; // from
const int move_toMask       = 0b0000000000000111111000000000; // to
const int move_c1Mask       = 0b0000000000111000000000000000; // tends to be first capture
const int move_c2Mask       = 0b0000000111000000000000000000; // tends to be second capture
const int move_c3Mask       = 0b0000111000000000000000000000; // tends to be third capture
const int move_c4Mask       = 0b0111000000000000000000000000; // tends to be fourth capture

const int move_kingcmask    = 0b1111000000000000000000000000;
const int move_kingc1mask   = 0b0001000000000000000000000000;
const int move_kingc2mask   = 0b0010000000000000000000000000;
const int move_kingc3mask   = 0b0100000000000000000000000000;
const int move_kingc4mask   = 0b1000000000000000000000000000;

const int move_cham_c_mask  = 0b0000011111111000000000000000;
const int move_cham_u_mask  = 0b0000000000001000000000000000;
const int move_cham_l_mask  = 0b0000000000010000000000000000;
const int move_cham_r_mask  = 0b0000000000100000000000000000;
const int move_cham_d_mask  = 0b0000000001000000000000000000;
const int move_cham_d1_mask = 0b0000000010000000000000000000;
const int move_cham_d2_mask = 0b0000000100000000000000000000;
const int move_cham_q_mask  = 0b0000001000000000000000000000;
const int move_cham_n_mask  = 0b0000010000000000000000000000;

const int move_captMask     = 0b1111111111111000000000000000; // all capture bits

const U64 straddlerBounds[] = {
    18446744073709486080ULL,// up
    18229723555195321596ULL,// left
    4557430888798830399ULL, // right
    281474976710655ULL      // down
};

int generateMoves(Move *movelist, int capturesOnly)
{
    int size = 0;

    // get squares that enemy immobilizer is not influencing
    U64 enemImm = position[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // all piece occupancy
    U64 totalBoard = position[white] | position[black];

    // all straddler moves
    U64 straddlers = position[toPlay + straddler] & notImmInfl;

    // these boards determine potential straddler captures
    // first segment is generic straddler rule.
    // second segment is chameleon rule.
    U64 straddlerUpBoard =
        (position[notToPlay] << 8) & (position[toPlay + straddler] << 16) |
        (position[notToPlay + straddler] << 8) & (position[toPlay + chameleon] << 16); // move any pieces down to match straddler board
    U64 straddlerLeftBoard = (
        (position[notToPlay] << 1) & (position[toPlay + straddler] << 2) |
        (position[notToPlay + straddler] << 1) & (position[toPlay + chameleon] << 2)
    ) & straddlerBounds[1];
    U64 straddlerRightBoard = (
        (position[notToPlay] >> 1) & (position[toPlay + straddler] >> 2) |
        (position[notToPlay + straddler] >> 1) & (position[toPlay + chameleon] >> 2)
    ) & straddlerBounds[2];
    U64 straddlerDownBoard =
        (position[notToPlay] >> 8) & (position[toPlay + straddler] >> 16) |
        (position[notToPlay + straddler] >> 8) & (position[toPlay + chameleon] >> 16);

    // combine all boards together to form the CAPTURE BOARD
    U64 straddlerCaptureBoard = straddlerUpBoard | straddlerLeftBoard | straddlerRightBoard | straddlerDownBoard;

    while (straddlers)
    {
        int sq = pop_lsb(straddlers);

        // generate moves
        U64 moves = get_rook_attacks(sq, totalBoard) & ~totalBoard;

        U64 straddlerCaptures = moves & straddlerCaptureBoard;

        if (!capturesOnly)
        {
            size += generateStraddlerMoves(sq, moves & ~straddlerCaptures, &movelist[size]);
        }

        // also add the captures right here. right now. just to be able to use the up/left/right/down straddler board from before.
        while (straddlerCaptures)
        {
            int to = pop_lsb(straddlerCaptures);
            U64 toBoard = 1ULL << to;

            Move move = (to << 9) | (sq << 3) | straddler;

            // consider captures...
            // up
            move |= (pieceList[to - 8] * ((straddlerUpBoard & toBoard) > 0)) << 15;
            // left
            move |= (pieceList[to - 1] * ((straddlerLeftBoard & toBoard) > 0)) << 18;
            // right
            move |= (pieceList[to + 1] * ((straddlerRightBoard & toBoard) > 0)) << 21;
            // down
            move |= (pieceList[to + 8] * ((straddlerDownBoard & toBoard) > 0)) << 24;

            movelist[size++] = move;
            straddlerCaptures &= straddlerCaptures - 1;
        }

        straddlers &= straddlers - 1;
    }

    // immobilizer moves (there's only one)
    if (!capturesOnly)
    {
        // enemy chameleon can immobilize an immobilizer...
        U64 chamBoard = position[notToPlay + chameleon];
        U64 chamBoard2 = chamBoard - 1 & chamBoard;

        int cham1 = pop_lsb(chamBoard);
        int cham2 = pop_lsb(chamBoard2);

        U64 chamInfl = (chamBoard > 0) * kingMoves[cham1] | (chamBoard2 > 0) * kingMoves[cham2];

        // now generate immobilizer moves
        U64 immBoard = position[toPlay + immobilizer] & notImmInfl & ~chamInfl;
        int sq = pop_lsb(immBoard);
        U64 moves = (immBoard > 0) * (
            get_rook_attacks(sq, totalBoard) |
            get_bishop_attacks(sq, totalBoard)
        ) & ~totalBoard;
        size += generateImmobilizerMoves(sq, moves, &movelist[size]);
    }

    // coordinator moves (there's only one)
    U64 coordBoard = position[toPlay + coordinator] & notImmInfl;
    if (coordBoard)
    {
        int sq = pop_lsb(coordBoard);
        U64 moves = (coordBoard > 0) * (
            get_rook_attacks(sq, totalBoard) |
            get_bishop_attacks(sq, totalBoard)
        ) & ~totalBoard;

        int kingSq = pop_lsb(position[toPlay + king]);
        U64 kingFile = files[kingSq & 0b111] & position[notToPlay];
        U64 kingRank = ranks[kingSq >> 3] & position[notToPlay];

        // find all squares where if a coordinator got there it would capture a piece.
        U64 captures = 0ULL;
        while (kingRank)
        {
            captures |= files[pop_lsb(kingRank) & 0b111];
            kingRank &= kingRank - 1;
        }
        while (kingFile)
        {
            captures |= ranks[pop_lsb(kingFile) >> 3];
            kingFile &= kingFile - 1;
        }

        if (!capturesOnly)
        {
            size += generateCoordinatorMoves(sq, moves & ~captures, &movelist[size]);
        }
        size += generateCoordinatorCaptures(sq, moves & captures, &movelist[size]);
    }

    // king moves (there's only one)
    U64 kingBoard = position[toPlay + king] & notImmInfl;
    if (kingBoard)
    {
        int sq = pop_lsb(kingBoard);
        U64 moves = (kingBoard > 0) * kingMoves[sq] & ~position[toPlay];

        size += generateKingMoves(sq, moves, &movelist[size], capturesOnly);
    }

    // all springer moves
    U64 springers = position[toPlay + springer] & notImmInfl;
    while (springers)
    {
        int sq = pop_lsb(springers);

        // generate moves
        U64 moves =
            get_rook_attacks(sq, totalBoard) |
            get_bishop_attacks(sq, totalBoard);

        if (!capturesOnly)
        {
            size += generateSpringerMoves(sq, moves & ~totalBoard, &movelist[size]);
        }
        size += generateSpringerCaptures(sq, moves & position[notToPlay], &movelist[size]);

        springers &= springers - 1;
    }

    // retractor moves (there's only one per side)
    U64 retractorBoard = position[toPlay + retractor] & notImmInfl;
    if (retractorBoard)
    {
        int sq = pop_lsb(retractorBoard);
        U64 moves = (retractorBoard > 0) * (
            get_rook_attacks(sq, totalBoard) |
            get_bishop_attacks(sq, totalBoard)
        );

        if (!capturesOnly)
        {
            size += generateRetractorMoves(sq, moves & ~kingMoves[sq] & ~totalBoard, &movelist[size]);
        }

        // consider moves separately from (potential) captures.
        size += generateRetractorCaptures(sq, moves & kingMoves[sq] & ~totalBoard, &movelist[size], capturesOnly);
    }

    // chameleon moves
    U64 chameleons = position[toPlay + chameleon] & notImmInfl;

    // handle chameleon teaming up with the straddler (or other chameleons)
    U64 chamUpBoard = ((position[notToPlay + straddler] << 8) & ((position[toPlay + chameleon] | position[toPlay + straddler]) << 16));
    U64 chamLeftBoard = (
        ((position[notToPlay + straddler] << 1) & ((position[toPlay + chameleon] | position[toPlay + straddler]) << 2))
    ) & straddlerBounds[1];
    U64 chamRightBoard = (
        ((position[notToPlay + straddler] >> 1) & ((position[toPlay + chameleon] | position[toPlay + straddler]) >> 2))
    ) & straddlerBounds[2];
    U64 chamDownBoard = ((position[notToPlay + straddler] >> 8) & ((position[toPlay + chameleon] | position[toPlay + straddler]) >> 16));
    while (chameleons)
    {
        int sq = pop_lsb(chameleons);

        U64 rookMoves = get_rook_attacks(sq, totalBoard);
        U64 bishopMoves = get_bishop_attacks(sq, totalBoard);

        size += generateChameleonRookMoves(sq, rookMoves & ~totalBoard, &movelist[size], chamUpBoard, chamLeftBoard, chamRightBoard, chamDownBoard, capturesOnly);
        size += generateChameleonBishopMoves(sq, bishopMoves & ~totalBoard, &movelist[size], capturesOnly);
        size += generateChameleonSpringerCaptures(sq, (rookMoves | bishopMoves) & position[notToPlay + springer], &movelist[size]);

        chameleons &= chameleons - 1;
    }

    return size;
}

int generateStraddlerMoves(int sq, U64 moves, Move* movelist)
{
    int size = 0;
    
    // extract and consider each move
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | straddler;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateImmobilizerMoves(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    // surely there's a faster way, right?
    // immobilizers can never capture pieces.
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | immobilizer;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateCoordinatorMoves(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    // if a coordinator is on the board, there should always be a king as well.
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | coordinator;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateCoordinatorCaptures(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    // if a coordinator is on the board, there should always be a king as well.
    int kingSq = pop_lsb(position[toPlay + king]);
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | coordinator;

        // capture bits (death squares with king)
        // note: coordinator potentially teaming up with a chameleon to take the king is not considered here.
        // the king is not considered to be a capturable piece
        U64 death1 = deathSquares[kingSq][to][0];
        move |= (pieceList[pop_lsb(death1)] * ((position[notToPlay] & death1) > 0)) << 15;

        U64 death2 = deathSquares[kingSq][to][1];
        move |= (pieceList[pop_lsb(death2)] * ((position[notToPlay] & death2) > 0)) << 18;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateKingMoves(int sq, U64 moves, Move* movelist, int capturesOnly)
{
    int size = 0;

    // king can coordinate with the coordinator
    U64 coordBoard = position[toPlay + coordinator];
    int coordSq = pop_lsb(coordBoard);

    // king can also coordinate with the chameleon, but only against the coordinator
    U64 chamBoard = position[toPlay + chameleon];
    int cham1 = pop_lsb(chamBoard);
    int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | king;

        // capture by displacement
        move |= pieceList[to] << 15;

        // capture by coordinator death square
        U64 deathco1 = deathSquares[coordSq][to][0] * (coordBoard > 0);
        move |= (pieceList[pop_lsb(deathco1)] * ((position[notToPlay] & deathco1) > 0)) << 18;

        U64 deathco2 = deathSquares[coordSq][to][1] * (coordBoard > 0);
        move |= (pieceList[pop_lsb(deathco2)] * ((position[notToPlay] & deathco2) > 0)) << 21;

        // capture an enemy coordinator by coordinating with a chameleon, only against the coordinator
        // make sure a double capture doesn't happen (coordinator-king and chameleon-king both capture same coordinator)
        U64 deathch1 = deathSquares[cham1][to][0] * (chamBoard > 0);
        move |= (((position[notToPlay + coordinator] & deathch1) > 0) && (deathch1 != deathco1)) << 24;

        U64 deathch2 = deathSquares[cham1][to][1] * (chamBoard > 0);
        move |= (((position[notToPlay + coordinator] & deathch2) > 0) && (deathch2 != deathco2)) << 25;

        U64 death = deathSquares[cham2][to][0] * ((chamBoard - 1 & chamBoard) > 0);
        move |= (((position[notToPlay + coordinator] & death) > 0) && (death != deathco1 && death != deathch1)) << 26;

        death = deathSquares[cham2][to][1] * ((chamBoard - 1 & chamBoard) > 0);
        move |= (((position[notToPlay + coordinator] & death) > 0) && (death != deathco2 && death != deathch2)) << 27;

        movelist[size] = move;
        size += !capturesOnly || (move & move_captMask) > 0;
        moves &= moves - 1;
    }

    return size;
}

int generateSpringerMoves(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | springer;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateSpringerCaptures(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    while (moves)
    {
        int capturing = pop_lsb(moves);

        // determine where the springer lands
        int to = pop_lsb(springerLeaps[sq][capturing]);

        Move move = (pieceList[capturing] << 15) | (to << 9) | (sq << 3) | springer;

        movelist[size++] = move;
        moves &= moves - 1;

        // actually, don't count the move if it did not result in a capture
        size -= (springerLeaps[sq][capturing] & (position[white] | position[black])) > 0 || springerLeaps[sq][capturing] == 0;
    }

    return size;
}

int generateRetractorMoves(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | retractor;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateRetractorCaptures(int sq, U64 moves, Move* movelist, int capturesOnly)
{
    int size = 0;

    while (moves)
    {
        int to = pop_lsb(moves);

        // determine where retractor lands
        int capturing = pop_lsb(retractorCaptures[sq][to]);

        Move move = ((pieceList[capturing] << 15) * ((position[notToPlay] & retractorCaptures[sq][to]) > 0)) | (to << 9) | (sq << 3) | retractor;

        movelist[size] = move;
        size += !capturesOnly || (move & move_captMask) > 0;
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonRookMoves(int sq, U64 moves, Move* movelist, U64 straddlerUpBoard, U64 straddlerLeftBoard, U64 straddlerRightBoard, U64 straddlerDownBoard, int capturesOnly)
{
    int size = 0;

    U64 enemyCoordBoard = position[notToPlay + coordinator];

    int kingSq = pop_lsb(position[toPlay + king]);

    // extract and consider each move
    while (moves)
    {
        int to = pop_lsb(moves);
        U64 toBoard = 1ULL << to;
        Move move = (to << 9) | (sq << 3) | chameleon;

        // each rook direction
        // consider captures...
        // up
        move |= ((straddlerUpBoard & toBoard) > 0) << 15;
        // left
        move |= ((straddlerLeftBoard & toBoard) > 0) << 16;
        // right
        move |= ((straddlerRightBoard & toBoard) > 0) << 17;
        // down
        move |= ((straddlerDownBoard & toBoard) > 0) << 18;

        // also consider the possibility of this being a retractor move
        // determine where retractor lands
        int capturing = pop_lsb(retractorCaptures[sq][to]);
        move |= move_cham_q_mask * ((position[notToPlay + retractor] & retractorCaptures[sq][to]) > 0);

        // coordinator moves
        move |= move_cham_d1_mask * ((deathSquares[to][kingSq][0] & enemyCoordBoard) > 0);
        move |= move_cham_d2_mask * ((deathSquares[to][kingSq][1] & enemyCoordBoard) > 0);

        movelist[size] = move;
        size += !capturesOnly || (move & move_captMask) > 0;
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonBishopMoves(int sq, U64 moves, Move* movelist, int capturesOnly)
{
    int size = 0;

    U64 enemyCoordBoard = position[notToPlay + coordinator];

    int kingSq = pop_lsb(position[toPlay + king]);

    // extract and consider each move
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | chameleon;

        // determine where retractor lands
        int capturing = pop_lsb(retractorCaptures[sq][to]);
        move |= move_cham_q_mask * ((position[notToPlay + retractor] & retractorCaptures[sq][to]) > 0);

        // coordinator moves
        move |= move_cham_d1_mask * ((deathSquares[to][kingSq][0] & enemyCoordBoard) > 0);
        move |= move_cham_d2_mask * ((deathSquares[to][kingSq][1] & enemyCoordBoard) > 0);

        movelist[size] = move;
        size += !capturesOnly || (move & move_captMask) > 0;
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonSpringerCaptures(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    // extract and consider each move
    while (moves)
    {
        int capturing = pop_lsb(moves);

        // determine where the springer lands
        int to = pop_lsb(springerLeaps[sq][capturing]);

        Move move = move_cham_n_mask | (to << 9) | (sq << 3) | chameleon;

        movelist[size++] = move;
        moves &= moves - 1;

        // actually, don't count the move if it did not result in a capture
        size -= (springerLeaps[sq][capturing] & (position[white] | position[black])) > 0 || springerLeaps[sq][capturing] == 0;
    }

    return size;
}

void printMove(Move move)
{
    printf("%s%s ", squareNames[(move >> 3) & 0b111111], squareNames[(move >> 9) & 0b111111]);
}

void prettyPrintMove(Move m)
{
    // get correct piece type
    switch(m & move_typeMask)
    {
        case straddler:
            printf("Straddler ");
            break;
        case retractor:
            printf("Retractor ");
            break;
        case king:
            printf("King ");
            break;
        case coordinator:
            printf("Coordinator ");
            break;
        case immobilizer:
            printf("Immobilizer ");
            break;
        case chameleon:
            printf("Chameleon ");
            break;
        case springer:
            printf("Springer ");
            break;
        default:
            printf("Unknown piece type ");
    }

    // print movement
    printf("moves from %s to %s ", squareNames[(m >> 3) & 0b111111], squareNames[(m >> 9) & 0b111111]);

    switch(m & move_typeMask)
    {
        case straddler:
        case coordinator:
        case immobilizer:
        case springer:
        case retractor:
            printf("with capture of %d %d %d %d", (m & move_c1Mask) >> 15, (m & move_c2Mask) >> 18, (m & move_c3Mask) >> 21, (m & move_c4Mask) >> 24);
            break;
        case chameleon:
            printf("with capture of %du %dl %dr %dd %dd1 %dd2 %dq %dn", (m & move_cham_u_mask) >> 15, (m & move_cham_l_mask) >> 16, (m & move_cham_r_mask) >> 17, (m & move_cham_d_mask) >> 18, (m & move_cham_d1_mask) >> 19, (m & move_cham_d2_mask) >> 20, (m & move_cham_q_mask) >> 21, (m & move_cham_n_mask) >> 22);
            break;
        case king:
            printf("with capture of %d %d %d %d", (m & move_c1Mask) >> 15, (m & move_c2Mask) >> 18, (m & move_c3Mask) >> 21, ((m & move_kingcmask) > 0) * coordinator);
            break;
        default:
            printf("???");
    }
    printf("\n");
}
