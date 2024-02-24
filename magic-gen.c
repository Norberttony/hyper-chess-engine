
// generates magic numbers for sliding move generation

#include <stdio.h>
#include <string.h>

#include "bitboard-utility.h"

#define MAX_BOARD_INDEX 100000

U64 rookMagics[64];
U64 bishopMagics[64];

U64 rookDefenderLookup[64];
U64 rookAttacks[MAX_BOARD_INDEX];

// to prevent wrapping around bitboard
const U64 not_8_rank = 18446744073709551360ULL;
const U64 not_1_rank =    72057594037927935ULL;
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file =  9187201950435737471ULL;

// populates the rookAttacksEmpty array
void genRookDefenderLookup();

// assumes that sq is not set to 1 in blockers
U64 genRookAttacks(int sq, U64 blockers);

int testRookMagicNumber(int number);

int main()
{

    genRookDefenderLookup();

    printBitboard(genRookAttacks(e4, 0ULL));

    // time to test every magic number in known existence :)
    //for (U64 i = 0; i < -1; i++)
    {

    }

    return 0;
}

int testRookMagicNumber(int number)
{
    // test it for every square
    for (int s = 0; s < 64; s++)
    {
        // zero out the rookAttacks array
        // Courtesy of
        // https://stackoverflow.com/questions/9146395/reset-c-int-array-to-zero-the-fastest-way
        memset(rookAttacks, 0, sizeof(rookAttacks));

        // determine the bit indexes for this position
        int bitIndexes[20] = {0};

        // try every defender combination
        U64 maxDefenders = rookDefenderLookup[s];
        
    }
}

void genRookDefenderLookup()
{
    U64 defenders = (~not_a_file << 1) | (~not_1_rank >> 8) | (~not_8_rank << 8) | (~not_h_file >> 1);
    for (int s = 0; s < 64; s++)
    {
        rookDefenderLookup[s] = genRookAttacks(s, defenders);
    }
}

U64 genRookAttacks(int sq, U64 blockers)
{
    U64 attacks = 0;
    U64 iterSq = 1ULL << sq;

    // keep looping while no blocker and not about to wrap around the board
    // to the right
    while (!(blockers & attacks) && iterSq & not_h_file)
    {
        iterSq <<= 1;
        attacks |= iterSq;
    }
    // so that all future loops do not think they are immediately attacking a piece
    blockers &= ~attacks;

    // to the left
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_a_file)
    {
        iterSq >>= 1;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // up
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_1_rank)
    {
        iterSq >>= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // down
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_8_rank)
    {
        iterSq <<= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    return attacks;
}
