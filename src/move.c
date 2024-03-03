
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

const int move_typeMask = 0b000000000000000000000000111; // piece performing move
const int move_fromMask = 0b000000000000000000111111000; // from
const int move_toMask   = 0b000000000000111111000000000; // to
const int move_c1Mask   = 0b000000000111000000000000000; // tends to be first capture
const int move_c2Mask   = 0b000000111000000000000000000; // tends to be second capture
const int move_c3Mask   = 0b000111000000000000000000000; // tends to be third capture
const int move_c4Mask   = 0b111000000000000000000000000; // tends to be fourth capture
const int move_captMask = 0b111111111111000000000000000; // all capture bits

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

    // all piece occupancy
    U64 totalBoard = position[white] | position[black];

    // all straddler moves
    U64 straddlers = position[toPlay + straddler];
    while (straddlers)
    {
        int sq = pop_lsb(straddlers);

        // generate moves
        U64 moves = rookAttacks[sq][((rookMasks[sq] & totalBoard) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])] & ~totalBoard;
        //bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])];

        generateStraddlerMoves(sq, moves, list);

        straddlers &= straddlers - 1;
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
    printf("moves from %s to %s with capture of %d\n", squareNames[(m >> 3) & 0b111111], squareNames[(m >> 9) & 0b111111], (m >> 15) & 0b111111111111);
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
    }
}
