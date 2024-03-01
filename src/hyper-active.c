
#include <stdio.h>

#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "defines.h"

// ranks are in reverse order, [0] accesses 8th rank, [1] accesses 7th, etc.
U64 ranks[8];
U64 files[8];

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file =  9187201950435737471ULL;

U64 kingMoves[64];
U64 deathSquares[64][64];

// populates ranks and files arrays (bitboards set to 1 if on either rank/file)
void populateRanksAndFiles();

// sq1 and sq2 are locations of king/coordinator, doesn't matter which is which.
U64 genDeathSquares(int sq1, int sq2);

// populates deathSquares lookup table
void populateDeathSquares();

// generates moves for a king
U64 genKingMoves(int sqIndex);

// populates king move lookup table
void populateKingMoves();

int main(void)
{
    populateKingMoves();
    populateRanksAndFiles(); // in order to use genDeathSquares (used by populateDeathSquares)
    populateDeathSquares();

    printBitboard(genDeathSquares(h8, h1));

    // initial board set up
    loadFEN(StartingFEN);

    // print position
    prettyPrintBoard();

    getchar();

    return 0;
}

void populateRanksAndFiles()
{
    for (int i = 0; i < 8; i++)
    {
        // get ith rank
        U64 rank = 0ULL;
        for (int f = 0; f < 8; f++)
        {
            rank |= 1ULL << (i * 8 + f);
        }
        ranks[i] = rank;

        // get ith file
        U64 file = 0ULL;
        for (int r = 0; r < 8; r++)
        {
            file |= 1ULL << (r * 8 + i);
        }
        files[i] = file;
    }
}

U64 genDeathSquares(int sq1, int sq2)
{
    // get files and ranks of both squares
    int f1 = sq1 % 8;
    int r1 = sq1 / 8;

    int f2 = sq2 % 8;
    int r2 = sq2 / 8;
    
    // if on same file/rank, no death squares
    if (f1 == f2 || r1 == r2)
    {
        return 0ULL;
    }

    // use ranks and files bitboard to find intersections
    return ranks[r1] & files[f2] | (ranks[r2] & files[f1]);
}

void populateDeathSquares()
{
    for (int s1 = 0; s1 < 64; s1++)
    {
        for (int s2 = 0; s2 < 64; s2++)
        {
            deathSquares[s1][s2] = genDeathSquares(s1, s2);
        }
    }
}

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
