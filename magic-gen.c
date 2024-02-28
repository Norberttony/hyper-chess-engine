
// generates magic numbers for sliding move generation

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bitboard-utility.h"

#define MAX_BOARD_INDEX 1024

U64 rookMagics[64];
U64 bishopMagics[64];

U64 rookMasks[64];
int rookMaskBitCount[64];
U64 rookAttacks[MAX_BOARD_INDEX];

// determines the smallest number of indexes necessary for a given square
int rookSmallest[64] = {MAX_BOARD_INDEX};

// to prevent wrapping around bitboard
const U64 not_8_rank = 18446744073709551360ULL;
const U64 not_1_rank =    72057594037927935ULL;
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file =  9187201950435737471ULL;

// populates the rookAttacksEmpty array
void genRookMasks();

// assumes that sq is not set to 1 in blockers
U64 genRookAttacks(int sq, U64 blockers);

// returns a random U64 number assuming that RAND_MAX is the largest 32 bit integer
U64 randomU64();

int testRookMagicNumber(int s, U64 number);

void displayTableStats();

int main()
{
    // initialize rookSmallest
    for (int s = 0; s < 64; s++)
    {
        rookSmallest[s] = MAX_BOARD_INDEX;
    }

    // seed random number generator
    srand(time(NULL));

    // generate rook masks
    printf("Generating rook masks...\n");
    genRookMasks();

    // search for magics!!!
    printf("Looking for magics...\n");

    // time to test every magic number in known existence :)
    for (int i = 0; i < 1000; i++)
    {
        U64 toTest = randomU64() & randomU64() & randomU64();

        for (int sq = 0; sq < 64; sq++)
        {
            testRookMagicNumber(sq, toTest);
        }
    }

    // show current size of table and all of the rook magics
    displayTableStats();

    return 0;
}

int testRookMagicNumber(int s, U64 number)
{
    // zero out the rookAttacks array
    // Courtesy of
    // https://stackoverflow.com/questions/9146395/reset-c-int-array-to-zero-the-fastest-way
    memset(rookAttacks, 0, sizeof(rookAttacks));

    U64 maxDefenders = rookMasks[s];

    // get all of the bit indexes on defender mask
    int bitIndexes[20] = {0};
    int bits = 0;
    while (maxDefenders)
    {
        int index = pop_lsb(maxDefenders);
        bitIndexes[bits++] = index;
        maxDefenders ^= 1ULL << index;
    }
    
    // try every defender combination
    int success = 1;
    int smallest = -1;
    for (int c = 0; c < (2 << rookMaskBitCount[s]); c++)
    {
        // generate test mask given bit indexes
        U64 testMask = 0ULL;
        for (int b = 0; b < bits; b++)
        {
            testMask |= (1ULL << bitIndexes[b]) * ((c & (1ULL << b)) > 0);
        }

        // use the testMask to test the magic number
        int testIndex = (testMask * number) >> (64 - bits);
        U64 attacks = genRookAttacks(s, testMask);

        // if this won't make the table smaller, don't bother
        // or if it even exceeds the max index
        if (rookSmallest[s] < testIndex || testIndex >= MAX_BOARD_INDEX)
        {
            success = 0;
            break;
        }

        // is this a unique index OR a constructive collision?
        if (!rookAttacks[testIndex] || rookAttacks[testIndex] == attacks)
        {
            rookAttacks[testIndex] = attacks;
        }
        else
        {
            success = 0;
            break;
        }

        // keep looking for how small the table will be given how big the array for this square
        // is.
        if (testIndex > smallest)
        {
            smallest = testIndex;
        }
    }

    if (success)
    {
        rookMagics[s] = number;
        rookSmallest[s] = smallest;

        printf("New rook magic number for square %d: %llu\n", s, number);
    }

    return 1;
}

void displayTableStats()
{
    // get the smallest board index necessary
    int smallest = MAX_BOARD_INDEX;
    int valid = 0;
    for (int s = 0; s < 64; s++)
    {
        if (rookSmallest[s] < smallest)
        {
            smallest = rookSmallest[s];
        }
        valid += rookMagics[s] > 0;
    }
    if (valid == 64)
    {
        printf("New rook table size:\n");
        printf("%d bytes\n", smallest * 64 * sizeof(U64));
    }
    else
    {
        printf("Rook table is still missing %d square(s)\n", 64 - valid);
    }

    // now print all rook magics
    printf("U64 rookMagics[64] =\n");
    printf("{\n");
    for (int i = 0; i < 64; i++)
    {
        printf("\t%lluULL,\n", rookMagics[i]);
    }
    printf("}\n");
}

void genRookMasks()
{
    
    for (int s = 0; s < 64; s++)
    {
        // rank and file of square
        int rank = s / 8;
        int file = s % 8;

        // create relevant occupancy mask for a rook at this square (in each direction excluding last square)
        U64 mask = 0ULL;
        for (int f = file; f > 0; f--)
        {
            mask |= 1ULL << (f + rank * 8);
        }

        for (int f = file; f < 7; f++)
        {
            mask |= 1ULL << (f + rank * 8);
        }

        for (int r = rank; r > 0; r--)
        {
            mask |= 1ULL << (file + r * 8);
        }

        for (int r = rank; r < 7; r++)
        {
            mask |= 1ULL << (file + r * 8);
        }

        // rook cannot capture where it currently is
        mask ^= 1ULL << s;

        // generate attacks and send them to the lookup table
        rookMasks[s] = mask;

        // count number of defenders
        while (mask)
        {
            rookMaskBitCount[s]++;
            mask ^= mask & -mask;
        }
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
    while (!(blockers & attacks) && iterSq & not_8_rank)
    {
        iterSq >>= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // down
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_1_rank)
    {
        iterSq <<= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    return attacks;
}

U64 randomU64()
{
    return ((U64)(rand() & 0xFFFF) << 48) | ((U64)(rand() & 0xFFFF) << 32) | ((U64)(rand() & 0xFFFF) << 16) | (U64)(rand() & 0xFFFF);
}
