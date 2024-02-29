
#include <stdio.h>

#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "defines.h"

const U64 ranks[8];
const U64 files[8];

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file =  9187201950435737471ULL;

U64 kingMoves[64];

U64 genKingMoves(int sqIndex)
{
    U64 king = 1ULL << sqIndex;
    
    U64 attacks = 0;

    attacks |= king << 1;
    attacks |= king << 7;
    attacks |= king << 8;
    attacks |= king << 9;

    attacks |= king >> 1;
    attacks |= king >> 7;
    attacks |= king >> 8;
    attacks |= king >> 9;

    // check file to see if we have to factor in out of bounds
    if (sqIndex % 8 == 7)
    {
        attacks &= not_a_file;
    }
    else if (sqIndex % 8 == 0)
    {
        attacks &= not_h_file;
    }

    return attacks;
}

void populateKingMoves()
{
    for (int s = 0; s < 64; s++)
    {
        kingMoves[s] = genKingMoves(s);
    }
}

int main(void)
{
    populateKingMoves();

    // initial board set up
    loadFEN(StartingFEN);

    // print retractor bitboard
    prettyPrintBoard();

    getchar();

    return 0;
}
