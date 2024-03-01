
#include "move.h"

Move moveList[300];
int moveListSize = 0;

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


void generateMoves()
{
    moveListSize = 0;

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

        // extract and consider each move
        while (moves)
        {
            int to = pop_lsb(moves);

            Move move = (to << 9) | (sq << 3) | straddler;

            prettyPrintMove(move);

            moveList[moveListSize++] = move;
            moves &= moves - 1;
        }

        straddlers &= straddlers - 1;
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
