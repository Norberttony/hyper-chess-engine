
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
const int move_captMask     = 0b1111111111111000000000000000; // all capture bits

int rookOffsets[] = {-8, -1, 1, 8};
U64 straddlerBounds[] = {
    18446744073709486080ULL,// up
    18229723555195321596ULL,// left
    4557430888798830399ULL, // right
    281474976710655ULL      // down
};

struct MoveList* generateMoves()
{
    struct MoveList *list = (struct MoveList*)malloc(sizeof(struct MoveList));
    list->size = 0;

    // get squares that enemy immobilizer is not influencing
    U64 enemImm = position[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // all piece occupancy
    U64 totalBoard = position[white] | position[black];

    // all straddler moves
    U64 straddlers = position[toPlay + straddler] & notImmInfl;
    while (straddlers)
    {
        int sq = pop_lsb(straddlers);

        // generate moves
        U64 moves = rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] & ~totalBoard;
        //bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])];

        generateStraddlerMoves(sq, moves, list);

        straddlers &= straddlers - 1;
    }

    // immobilizer moves (there's only one)
    {
        U64 immBoard = position[toPlay + immobilizer] & notImmInfl;
        int sq = pop_lsb(immBoard);
        U64 moves = (immBoard > 0) * (
            rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] |
            bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])]
        ) & ~totalBoard;
        generateImmobilizerMoves(sq, moves, list);
    }

    // coordinator moves (there's only one)
    {
        U64 coordBoard = position[toPlay + coordinator] & notImmInfl;
        int sq = pop_lsb(coordBoard);
        U64 moves = (coordBoard > 0) * (
            rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] |
            bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])]
        ) & ~totalBoard;
        generateCoordinatorMoves(sq, moves, list);
    }

    // king moves (there's only one)
    {
        U64 kingBoard = position[toPlay + king] & notImmInfl;
        int sq = pop_lsb(kingBoard);
        U64 moves = (kingBoard > 0) * kingMoves[sq] & ~position[toPlay];
        generateKingMoves(sq, moves, list);
    }

    // all springer moves
    U64 springers = position[toPlay + springer] & notImmInfl;
    while (springers)
    {
        int sq = pop_lsb(springers);

        // generate moves
        U64 moves =
            rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] |
            bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])];

        generateSpringerMoves(sq, moves & ~totalBoard, list);
        generateSpringerCaptures(sq, moves & position[notToPlay], list);

        springers &= springers - 1;
    }

    // retractor moves (there's only one per side)
    {
        U64 retractorBoard = position[toPlay + retractor] & notImmInfl;
        int sq = pop_lsb(retractorBoard);
        U64 moves = (retractorBoard > 0) * (
            rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] |
            bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])]
        );

        // consider moves separately from (potential) captures.
        generateRetractorMoves(sq, moves & ~kingMoves[sq] & ~totalBoard, list);
        generateRetractorCaptures(sq, kingMoves[sq] & ~totalBoard, list);        
    }

    return list;
}

void generateStraddlerMoves(int sq, U64 moves, struct MoveList* movelist)
{
    // extract and consider each move
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | straddler;

        // each rook direction
        for (int d = 0; d < 4; d++)
        {
            // represents U L R or D square from current square
            U64 dirBoard = 1ULL << (to + rookOffsets[d]);

            // straddler out of bounds if capture will wrap around board
            // also check if there's an enemy one square away
            if (!(straddlerBounds[d] & (1ULL << to)) || !(position[notToPlay] & dirBoard))
            {
                continue;
            }

            U64 dDirBoard = 1ULL << (to + 2 * rookOffsets[d]);
            // must team up with straddler
            if (position[toPlay + straddler] & dDirBoard)
            {
                move |= pieceList[to + rookOffsets[d]] << (15 + d * 3);
            }
            else
            {
                // special chameleon rule only against straddlers
                move |= (position[notToPlay + straddler] & dirBoard && position[toPlay + chameleon] & dDirBoard) << (15 + d * 3);
            }
        }

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateImmobilizerMoves(int sq, U64 moves, struct MoveList* movelist)
{
    // surely there's a faster way, right?
    // immobilizers can never capture pieces.
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | immobilizer;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateCoordinatorMoves(int sq, U64 moves, struct MoveList* movelist)
{
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

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateKingMoves(int sq, U64 moves, struct MoveList* movelist)
{
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
        U64 death = deathSquares[cham1][to][0] * (chamBoard > 0);
        move |= (pieceList[pop_lsb(death)] * (position[notToPlay + coordinator] & death > 0) * (death != deathco1)) << 24;

        death = deathSquares[cham1][to][1] * (chamBoard > 0);
        move |= (pieceList[pop_lsb(death)] * (position[notToPlay + coordinator] & death > 0) * (death != deathco2)) << 25;

        death = deathSquares[cham2][to][0] * ((chamBoard - 1 & chamBoard) > 0);
        move |= (pieceList[pop_lsb(death)] * (position[notToPlay + coordinator] & death > 0) * (death != deathco1)) << 26;

        death = deathSquares[cham2][to][1] * ((chamBoard - 1 & chamBoard) > 0);
        move |= (pieceList[pop_lsb(death)] * (position[notToPlay + coordinator] & death > 0) * (death != deathco2)) << 27;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateSpringerMoves(int sq, U64 moves, struct MoveList* movelist)
{
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | springer;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateSpringerCaptures(int sq, U64 moves, struct MoveList* movelist)
{
    while (moves)
    {
        int capturing = pop_lsb(moves);

        // determine where the springer lands
        int to = pop_lsb(springerLeaps[sq][capturing]);

        Move move = (pieceList[capturing] << 15) | (to << 9) | (sq << 3) | springer;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;

        // actually, don't count the move if it did not result in a capture
        movelist->size -= (springerLeaps[sq][capturing] & (position[white] | position[black])) > 0 || springerLeaps[sq][capturing] == 0;
    }
}

void generateRetractorMoves(int sq, U64 moves, struct MoveList* movelist)
{
    while (moves)
    {
        int to = pop_lsb(moves);
        Move move = (to << 9) | (sq << 3) | retractor;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void generateRetractorCaptures(int sq, U64 moves, struct MoveList* movelist)
{
    while (moves)
    {
        int to = pop_lsb(moves);

        // determine where retractor lands
        int capturing = pop_lsb(retractorCaptures[sq][to]);

        Move move = ((pieceList[capturing] << 15) * ((position[notToPlay] & retractorCaptures[sq][to]) > 0)) | (to << 9) | (sq << 3) | retractor;

        movelist->list[movelist->size++] = move;
        moves &= moves - 1;
    }
}

void prettyPrintMove(Move m)
{
    // get correct piece type
    switch(m & 0b111)
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

    switch(m & 0b111)
    {
        case straddler:
        case coordinator:
        case immobilizer:
        case springer:
        case retractor:
            printf("with capture of %d %d %d %d", (m & move_c1Mask) >> 15, (m & move_c2Mask) >> 18, (m & move_c3Mask) >> 21, (m & move_c4Mask) >> 24);
            break;
        case chameleon:
            printf("not implemented");
            break;
        case king:
            printf("with capture of %d %d %d %d", (m & move_c1Mask) >> 15, (m & move_c2Mask) >> 18, (m & move_c3Mask) >> 21, ((m & move_kingcmask) > 0) * coordinator);
            break;
        default:
            printf("???");
    }
    printf("\n");
}

void makeMove(Move m)
{
    // decode move
    int type = m & 0b111;
    int from = (m >> 3) & 0b111111;
    int to = (m >> 9) & 0b111111;

    int c1 = (m >> 15) & 0b111;
    int c2 = (m >> 18) & 0b111;
    int c3 = (m >> 21) & 0b111;
    int c4 = (m >> 24) & 0b111;

    int coordinateSq;

    // interpret capture bits
    switch(type)
    {
        case straddler:
            // up
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << (to - 8);
            position[notToPlay] ^= 1ULL * (c1 > 0) << (to - 8);
            pieceList[to - 8] = pieceList[to - 8] * (c1 == 0);

            // left
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << (to - 1);
            position[notToPlay] ^= 1ULL * (c2 > 0) << (to - 1);
            pieceList[to - 1] = pieceList[to - 1] * (c2 == 0);

            // right
            position[notToPlay + c3] ^= 1ULL * (c3 > 0) << (to + 1);
            position[notToPlay] ^= 1ULL * (c3 > 0) << (to + 1);
            pieceList[to + 1] = pieceList[to + 1] * (c3 == 0);

            // down
            position[notToPlay + c4] ^= 1ULL * (c4 > 0) << (to + 8);
            position[notToPlay] ^= 1ULL * (c4 > 0) << (to + 8);
            pieceList[to + 8] = pieceList[to + 8] * (c4 == 0);

            break;
        
        case immobilizer:
            break;

        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << top;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << top;
            pieceList[top] *= c1 == 0;

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << bottom;
            position[notToPlay]      ^= 1ULL * (c2 > 0) << bottom;
            pieceList[bottom] *= c2 == 0;

            break;

        case king:

            // can capture by displacement
            // assumes pieceList will be updated by king overwriting square
            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << to;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << to;

            // can form death squares with own coordinator
            coordinateSq = pop_lsb(position[toPlay + coordinator]);

            int deathSq = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c2] ^= 1ULL * (c2 > 0) << deathSq;
            position[notToPlay]      ^= 1ULL * (c2 > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (c2 == 0);

            deathSq = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c3] ^= 1ULL * (c3 > 0) << deathSq;
            position[notToPlay]      ^= 1ULL * (c3 > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (c3 == 0);

            // can form death squares with chameleons (only against coordinator)
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            // to-do: combine all of the boards into one and then apply them.
            int isDeath = m & move_kingc1mask;
            deathSq = pop_lsb(deathSquares[cham1][to][0]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (isDeath == 0);

            isDeath = m & move_kingc2mask;
            deathSq = pop_lsb(deathSquares[cham1][to][1]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (isDeath == 0);

            isDeath = m & move_kingc3mask;
            deathSq = pop_lsb(deathSquares[cham2][to][0]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (isDeath == 0);

            isDeath = m & move_kingc4mask;
            deathSq = pop_lsb(deathSquares[cham2][to][1]);
            position[notToPlay + coordinator] ^= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               ^= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] = pieceList[deathSq] * (isDeath == 0);

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] = pieceList[coordinateSq] * (c1 == 0);

            break;
        
        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            position[notToPlay + c1] ^= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      ^= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq]  *= (c1 == 0);

            break;
    }

    // move piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    position[toPlay + type] ^= toggle;
    position[toPlay] ^= toggle;

    // update piece list
    pieceList[to] = pieceList[from];
    pieceList[from] = 0;

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;
}

void unmakeMove(Move m)
{
    // decode move
    int type = m & 0b111;
    int from = (m >> 3) & 0b111111;
    int to = (m >> 9) & 0b111111;

    int c1 = (m >> 15) & 0b111;
    int c2 = (m >> 18) & 0b111;
    int c3 = (m >> 21) & 0b111;
    int c4 = (m >> 24) & 0b111;

    int coordinateSq;

    // toggle turn
    toPlay = !toPlay * 8;
    notToPlay = !notToPlay * 8;

    // unmove piece
    U64 toggle = (1ULL << from) | (1ULL << to);
    position[toPlay + type] ^= toggle;
    position[toPlay] ^= toggle;

    // update piece list
    pieceList[from] = pieceList[to];
    pieceList[to] = 0;

    // interpret capture bits
    switch(type)
    {
        case straddler:
            // up
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << (to - 8);
            position[notToPlay] |= 1ULL * (c1 > 0) << (to - 8);
            pieceList[to - 8] += c1; // assumes that, after a capture, the piece at this square would be gone.

            // left
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << (to - 1);
            position[notToPlay] |= 1ULL * (c2 > 0) << (to - 1);
            pieceList[to - 1] += c2;

            // right
            position[notToPlay + c3] |= 1ULL * (c3 > 0) << (to + 1);
            position[notToPlay] |= 1ULL * (c3 > 0) << (to + 1);
            pieceList[to + 1] += c3;

            // down
            position[notToPlay + c4] |= 1ULL * (c4 > 0) << (to + 8);
            position[notToPlay] |= 1ULL * (c4 > 0) << (to + 8);
            pieceList[to + 8] += c4;
            break;

        case immobilizer:
            break;
        
        case coordinator:
            coordinateSq = pop_lsb(position[toPlay + king]);
            
            // top death square
            int top = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << top;
            position[notToPlay]      |= 1ULL * (c1 > 0) << top;
            pieceList[top] += c1;

            // bottom death square
            int bottom = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << bottom;
            position[notToPlay]      |= 1ULL * (c2 > 0) << bottom;
            pieceList[bottom] += c2;
            
            break;

        case king:

            // capture by displacement
            position[notToPlay + c1] |= 1ULL * (c1 > 0) << to;
            position[notToPlay]      |= 1ULL * (c1 > 0) << to;
            pieceList[to] += c1;

            // death squares with coordinator
            // can form death squares with own coordinator
            coordinateSq = pop_lsb(position[toPlay + coordinator]);

            int deathSq = pop_lsb(deathSquares[coordinateSq][to][0]);
            position[notToPlay + c2] |= 1ULL * (c2 > 0) << deathSq;
            position[notToPlay]      |= 1ULL * (c2 > 0) << deathSq;
            pieceList[deathSq] += c2;

            deathSq = pop_lsb(deathSquares[coordinateSq][to][1]);
            position[notToPlay + c3] |= 1ULL * (c3 > 0) << deathSq;
            position[notToPlay]      |= 1ULL * (c3 > 0) << deathSq;
            pieceList[deathSq] += c3;

            // consider king-chameleon duo
            U64 chamBoard = position[toPlay + chameleon];
            int cham1 = pop_lsb(chamBoard);
            int cham2 = pop_lsb(chamBoard - 1 & chamBoard);

            int isDeath = m & move_kingc1mask;
            deathSq = pop_lsb(deathSquares[cham1][to][0]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            isDeath = m & move_kingc2mask;
            deathSq = pop_lsb(deathSquares[cham1][to][1]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            isDeath = m & move_kingc3mask;
            deathSq = pop_lsb(deathSquares[cham2][to][0]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            isDeath = m & move_kingc4mask;
            deathSq = pop_lsb(deathSquares[cham2][to][1]);
            position[notToPlay + coordinator] |= 1ULL * (isDeath > 0) << deathSq;
            position[notToPlay]               |= 1ULL * (isDeath > 0) << deathSq;
            pieceList[deathSq] += coordinator * (isDeath > 0);

            break;

        case springer:

            // just a garbage variable... coordinateSq in this case is where the springer captured
            // a piece at
            coordinateSq = pop_lsb(springerCaptures[from][to]);

            position[notToPlay + c1] |= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      |= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] += c1;

            break;

        case retractor:

            // coordinateSq in this case is where the retractor captured a piece at
            coordinateSq = pop_lsb(retractorCaptures[from][to]);

            position[notToPlay + c1] |= 1ULL * (c1 > 0) << coordinateSq;
            position[notToPlay]      |= 1ULL * (c1 > 0) << coordinateSq;
            pieceList[coordinateSq] += c1;

            break;
    }
}
