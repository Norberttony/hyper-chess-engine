
#include "move.h"

const Move move_typeMask     = 0x7; // piece performing move
const Move move_fromMask     = 0x1F8; // from
const Move move_toMask       = 0x7E00; // to
const Move move_c1Mask       = 0x38000; // tends to be first capture
const Move move_c2Mask       = 0x1C0000; // tends to be second capture
const Move move_c3Mask       = 0xE00000; // tends to be third capture
const Move move_c4Mask       = 0x7000000; // tends to be fourth capture

const Move move_kingcmask    = 0xF000000;
const Move move_kingc1mask   = 0x1000000;
const Move move_kingc2mask   = 0x2000000;
const Move move_kingc3mask   = 0x4000000;
const Move move_kingc4mask   = 0x8000000;

const Move move_cham_c_mask  = 0x7F8000;
const Move move_cham_u_mask  = 0x008000;
const Move move_cham_l_mask  = 0x010000;
const Move move_cham_r_mask  = 0x020000;
const Move move_cham_d_mask  = 0x040000;
const Move move_cham_d1_mask = 0x080000;
const Move move_cham_d2_mask = 0x100000;
const Move move_cham_q_mask  = 0x200000;
const Move move_cham_n_mask  = 0x400000;

const Move move_captMask     = 0xFFF8000; // all capture bits

const U64 straddlerBounds[] = {
    18446744073709486080ULL,// up
    18229723555195321596ULL,// left
    4557430888798830399ULL, // right
    281474976710655ULL      // down
};

int generateMoves(Move *movelist, int capturesOnly)
{
    generatePins();
    int size = 0;

    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;

    U64* position = g_pos.boards;

    // get squares that enemy immobilizer is not influencing
    U64 enemImm = g_pos.boards[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // all piece occupancy
    U64 totalBoard = g_pos.boards[white] | g_pos.boards[black];

    // all straddler moves
    U64 straddlers = g_pos.boards[toPlay + straddler] & notImmInfl;

    // these boards determine potential straddler captures
    // first segment is generic straddler rule.
    // second segment is chameleon rule.
    U64 straddlerUpBoard =
        ((position[notToPlay] << 8) & (position[toPlay + straddler] << 16)) |
        ((position[notToPlay + straddler] << 8) & (position[toPlay + chameleon] << 16)); // move any pieces down to match straddler board
    U64 straddlerLeftBoard = (
        ((position[notToPlay] << 1) & (position[toPlay + straddler] << 2)) |
        ((position[notToPlay + straddler] << 1) & (position[toPlay + chameleon] << 2))
    ) & straddlerBounds[1];
    U64 straddlerRightBoard = (
        ((position[notToPlay] >> 1) & (position[toPlay + straddler] >> 2)) |
        ((position[notToPlay + straddler] >> 1) & (position[toPlay + chameleon] >> 2))
    ) & straddlerBounds[2];
    U64 straddlerDownBoard =
        ((position[notToPlay] >> 8) & (position[toPlay + straddler] >> 16)) |
        ((position[notToPlay + straddler] >> 8) & (position[toPlay + chameleon] >> 16));

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
            size += generateStraddlerMoves(sq, moves & ~straddlerCaptures & get_pin_mask(sq), &movelist[size]);
        }

        // also add the captures right here. right now. just to be able to use the up/left/right/down straddler board from before.
        while (straddlerCaptures)
        {
            int to = pop_lsb(straddlerCaptures);
            U64 toBoard = 1ULL << to;

            Move move = (to << 9) | (sq << 3) | straddler;

            // consider captures...
            // up
            move |= (g_pos.pieceList[to - 8] * ((straddlerUpBoard & toBoard) > 0)) << 15;
            // left
            move |= (g_pos.pieceList[to - 1] * ((straddlerLeftBoard & toBoard) > 0)) << 18;
            // right
            move |= (g_pos.pieceList[to + 1] * ((straddlerRightBoard & toBoard) > 0)) << 21;
            // down
            move |= (g_pos.pieceList[to + 8] * ((straddlerDownBoard & toBoard) > 0)) << 24;

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
        U64 chamBoard2 = (chamBoard - 1) & chamBoard;

        int cham1 = pop_lsb(chamBoard);
        int cham2 = pop_lsb(chamBoard2);

        U64 chamInfl = (chamBoard > 0) * kingMoves[cham1] | (chamBoard2 > 0) * kingMoves[cham2];

        // now generate immobilizer moves
        U64 immBoard = position[toPlay + immobilizer] & notImmInfl & ~chamInfl;
        int sq = pop_lsb(immBoard);
        U64 moves = (immBoard > 0) * (get_queen_attacks(sq, totalBoard)) & ~totalBoard;
        size += generateImmobilizerMoves(sq, moves & get_imm_pin_mask(sq), &movelist[size]);
    }

    // coordinator moves (there's only one)
    U64 coordBoard = position[toPlay + coordinator] & notImmInfl;
    if (coordBoard)
    {
        int sq = pop_lsb(coordBoard);
        U64 moves = (coordBoard > 0) * (get_queen_attacks(sq, totalBoard)) & ~totalBoard;

        int kingSq = pop_lsb(position[toPlay + king]);
        U64 kingFile = files[get_file(kingSq)] & position[notToPlay];
        U64 kingRank = ranks[get_rank(kingSq)] & position[notToPlay];

        // find all squares where if a coordinator got there it would capture a piece.
        U64 captures = 0ULL;
        while (kingRank)
        {
            captures |= files[get_file(pop_lsb(kingRank))];
            kingRank &= kingRank - 1;
        }
        while (kingFile)
        {
            captures |= ranks[get_rank(pop_lsb(kingFile))];
            kingFile &= kingFile - 1;
        }

        if (!capturesOnly)
        {
            size += generateCoordinatorMoves(sq, moves & ~captures & get_pin_mask(sq), &movelist[size]);
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
        U64 moves = get_queen_attacks(sq, totalBoard);

        if (!capturesOnly)
        {
            size += generateSpringerMoves(sq, moves & ~totalBoard & get_pin_mask(sq), &movelist[size]);
        }
        size += generateSpringerCaptures(sq, moves & position[notToPlay], &movelist[size]);

        springers &= springers - 1;
    }

    // retractor moves (there's only one per side)
    U64 retractorBoard = position[toPlay + retractor] & notImmInfl;
    if (retractorBoard)
    {
        int sq = pop_lsb(retractorBoard);
        U64 moves = (retractorBoard > 0) * (get_queen_attacks(sq, totalBoard));

        if (!capturesOnly)
        {
            size += generateRetractorMoves(sq, moves & ~kingMoves[sq] & ~totalBoard & get_pin_mask(sq), &movelist[size]);
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

    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;

    // if a coordinator is on the board, there should always be a king as well.
    int kingSq = pop_lsb(g_pos.boards[toPlay + king]);
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | coordinator;

        // capture bits (death squares with king)
        // note: coordinator potentially teaming up with a chameleon to take the king is not considered here.
        // the king is not considered to be a capturable piece
        U64 death1 = deathSquares[kingSq][to][0];
        move |= (g_pos.pieceList[pop_lsb(death1)] * ((g_pos.boards[notToPlay] & death1) > 0)) << 15;

        U64 death2 = deathSquares[kingSq][to][1];
        move |= (g_pos.pieceList[pop_lsb(death2)] * ((g_pos.boards[notToPlay] & death2) > 0)) << 18;

        movelist[size++] = move;
        moves &= moves - 1;
    }

    return size;
}

int generateKingMoves(int sq, U64 moves, Move* movelist, int capturesOnly)
{
    int size = 0;

    int toPlay = g_pos.toPlay;
    int notToPlay = g_pos.notToPlay;

    // king can coordinate with the coordinator
    U64 coordBoard = g_pos.boards[toPlay + coordinator];
    int coordSq = pop_lsb(coordBoard);

    U64 enemCoordBoard = g_pos.boards[notToPlay + coordinator];

    // king can also coordinate with the chameleon, but only against the coordinator
    U64 chamBoard = g_pos.boards[toPlay + chameleon];
    int cham1 = pop_lsb(chamBoard);
    int cham2 = pop_lsb((chamBoard - 1) & chamBoard);

    U64 enemBoard = g_pos.boards[notToPlay];

    // since isSquareControlled and anything else in this routine does not use pieceList, we can
    // safely remove the king from the board. This will allow isSquareControlled to consider moves
    // that x-ray where the king currently is.
    U64 kingBoardCopy = g_pos.boards[toPlay | king];
    g_pos.boards[toPlay] ^= kingBoardCopy;
    g_pos.boards[toPlay | king] = 0ULL;

    U64 enemKC = g_pos.boards[g_pos.notToPlay | king] | g_pos.boards[g_pos.notToPlay | chameleon];

    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | king;

        // capture by displacement
        move |= g_pos.pieceList[to] << 15;

        // capture by coordinator death square
        U64 deathco1 = deathSquares[coordSq][to][0] * (coordBoard > 0);
        move |= (g_pos.pieceList[pop_lsb(deathco1)] * ((enemBoard & deathco1) > 0)) << 18;

        U64 deathco2 = deathSquares[coordSq][to][1] * (coordBoard > 0);
        move |= (g_pos.pieceList[pop_lsb(deathco2)] * ((enemBoard & deathco2) > 0)) << 21;

        // capture an enemy coordinator by coordinating with a chameleon, only against the coordinator
        // make sure a double capture doesn't happen (coordinator-king and chameleon-king both capture same coordinator)
        U64 deathch1 = deathSquares[cham1][to][0] * (chamBoard > 0);
        move |= (((enemCoordBoard & deathch1) > 0) && (deathch1 != deathco1)) << 24;

        U64 deathch2 = deathSquares[cham1][to][1] * (chamBoard > 0);
        move |= (((enemCoordBoard & deathch2) > 0) && (deathch2 != deathco2)) << 25;

        U64 death = deathSquares[cham2][to][0] * (((chamBoard - 1) & chamBoard) > 0);
        move |= (((enemCoordBoard & death) > 0) && (death != deathco1 && death != deathch1)) << 26;

        death = deathSquares[cham2][to][1] * (((chamBoard - 1) & chamBoard) > 0);
        move |= (((enemCoordBoard & death) > 0) && (death != deathco2 && death != deathch2)) << 27;

        movelist[size] = move;
        size += !capturesOnly || is_move_capt(move);
        if (!capturesOnly && !is_move_capt(move))
        {
            // now, isSquareControlled is kind of poorly written... I should fix this, but for now, any
            // king/chameleon takes by vanilla king move is not checked for when running isSquareControlled.
            if ((enemKC & kingMoves[to]) || isSquareControlled(g_pos.notToPlay, to, king))
            {
                size--;
            }
        }
        moves &= moves - 1;
    }

    g_pos.boards[toPlay] ^= kingBoardCopy;
    g_pos.boards[toPlay | king] = kingBoardCopy;

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

    U64 totalBoard = g_pos.boards[white] | g_pos.boards[black];

    while (moves)
    {
        int capturing = pop_lsb(moves);
        U64 land = springerLeaps[sq][capturing];

        // determine where the springer lands
        int to = pop_lsb(land);

        Move move = (g_pos.pieceList[capturing] << 15) | (to << 9) | (sq << 3) | springer;

        movelist[size++] = move;
        moves &= moves - 1;

        // actually, don't count the move if it did not result in a capture
        size -= (land & totalBoard) > 0 || land == 0;
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

    U64 pinMask = get_pin_mask(sq);

    while (moves)
    {
        int to = pop_lsb(moves);
        U64 toBoard = 1ULL << to;

        // determine where retractor lands
        int capturing = pop_lsb(retractorCaptures[sq][to]);

        Move move = ((g_pos.pieceList[capturing] << 15) * ((g_pos.boards[g_pos.notToPlay] & retractorCaptures[sq][to]) > 0)) | (to << 9) | (sq << 3) | retractor;

        movelist[size] = move;
        // only count quiet moves if we're generating all moves AND they obey pin masks.
        size += is_move_capt(move) || (!capturesOnly && (pinMask & toBoard));
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonRookMoves(int sq, U64 moves, Move* movelist, U64 straddlerUpBoard, U64 straddlerLeftBoard, U64 straddlerRightBoard, U64 straddlerDownBoard, int capturesOnly)
{
    int size = 0;

    U64 enemyCoordBoard = g_pos.boards[g_pos.notToPlay + coordinator];
    U64 enemyRetractorBoard = g_pos.boards[g_pos.notToPlay + retractor];
    int kingSq = pop_lsb(g_pos.boards[g_pos.toPlay + king]);
    U64 pinMask = get_pin_mask(sq);

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
        move |= move_cham_q_mask * ((enemyRetractorBoard & retractorCaptures[sq][to]) > 0);

        // coordinator moves
        move |= move_cham_d1_mask * ((deathSquares[to][kingSq][0] & enemyCoordBoard) > 0);
        move |= move_cham_d2_mask * ((deathSquares[to][kingSq][1] & enemyCoordBoard) > 0);

        movelist[size] = move;
        // only count quiet moves if we're generating all moves AND they obey pin masks.
        size += is_move_capt(move) || (!capturesOnly && (pinMask & toBoard));
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonBishopMoves(int sq, U64 moves, Move* movelist, int capturesOnly)
{
    int size = 0;

    U64 enemyCoordBoard = g_pos.boards[g_pos.notToPlay + coordinator];
    int kingSq = pop_lsb(g_pos.boards[g_pos.toPlay + king]);
    U64 pinMask = get_pin_mask(sq);

    // extract and consider each move
    while (moves)
    {
        int to = pop_lsb(moves);
        U64 toBoard = 1ULL << to;
        Move move = (to << 9) | (sq << 3) | chameleon;

        // determine where retractor lands
        move |= move_cham_q_mask * ((g_pos.boards[g_pos.notToPlay + retractor] & retractorCaptures[sq][to]) > 0);

        // coordinator moves
        move |= move_cham_d1_mask * ((deathSquares[to][kingSq][0] & enemyCoordBoard) > 0);
        move |= move_cham_d2_mask * ((deathSquares[to][kingSq][1] & enemyCoordBoard) > 0);

        movelist[size] = move;
        // only count quiet moves if we're generating all moves AND they obey pin masks.
        size += is_move_capt(move) || (!capturesOnly && (pinMask & toBoard));
        moves &= moves - 1;
    }

    return size;
}

int generateChameleonSpringerCaptures(int sq, U64 moves, Move* movelist)
{
    int size = 0;

    U64 totalBoard = g_pos.boards[white] | g_pos.boards[black];

    // extract and consider each move
    while (moves)
    {
        int capturing = pop_lsb(moves);
        U64 land = springerLeaps[sq][capturing];

        // determine where the springer lands
        int to = pop_lsb(land);

        Move move = move_cham_n_mask | (to << 9) | (sq << 3) | chameleon;

        movelist[size++] = move;
        moves &= moves - 1;

        // actually, don't count the move if it did not result in a capture
        size -= (land & totalBoard) > 0 || land == 0;
    }

    return size;
}

void printMove(Move move)
{
    printf("%s%s ", squareNames[get_from(move)], squareNames[get_to(move)]);
}

void prettyPrintMove(Move m)
{
    // get correct piece type
    switch(get_type(m))
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
    printf("moves from %s to %s ", squareNames[get_from(m)], squareNames[get_to(m)]);

    switch(get_type(m))
    {
        case straddler:
        case coordinator:
        case immobilizer:
        case springer:
        case retractor:
            printf("with capture of %d %d %d %d", get_c1(m), get_c2(m), get_c3(m), get_c4(m));
            break;
        case chameleon:
            printf("with capture of %du %dl %dr %dd %dd1 %dd2 %dq %dn", get_b_cu(m), get_b_cl(m), get_b_cr(m), get_b_cd(m), get_b_cd1(m), get_b_cd2(m), get_b_cq(m), get_b_cn(m));
            break;
        case king:
            printf("with capture of %d %d %d %d", get_c1(m), get_c2(m), get_c3(m), get_kb_c(m) * coordinator);
            break;
        default:
            printf("???");
    }
    printf("\n");
}
