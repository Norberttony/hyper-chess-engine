
// generates magic numbers for sliding move generation

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "bitboard-utility.h"
#include "magic-bitboards.h"

#define MAX_BOARD_INDEX 4096

U64 testAttacks[MAX_BOARD_INDEX];

// determines the smallest number of indexes necessary for a given square
int rookSmallest[64];
int bishopSmallest[64];

// populates the rookMasks array
void genRookMasks(void);
void genBishopMasks(void);

// returns a random U64 number assuming that RAND_MAX is the largest 32 bit integer
U64 randomU64(void);

int testMagicNumber(int s, U64 number, int isBishop);

void displayTableStats(void);

int main(void)
{
    // generate masks
    printf("Generating masks...\n");
    genRookMasks();
    genBishopMasks();

    // initialize smallest
    for (int s = 0; s < 64; s++)
    {
        rookSmallest[s] = MAX_BOARD_INDEX;
        bishopSmallest[s] = MAX_BOARD_INDEX;
        if (rookMagics[s])
        {
            // populates rookSmallest but... is kind of inefficient maybe? eh.
            if (!testMagicNumber(s, rookMagics[s], 0))
            {
                rookMagics[s] = 0ULL; // invalid magic!!
            }
        }
        if (bishopMagics[s])
        {
            if (!testMagicNumber(s, bishopMagics[s], 1))
            {
                bishopMagics[s] = 0ULL; // invalid magic!!
            }
        }
    }

    // seed random number generator
    srand(time(NULL));

    // search for magics!!!
    printf("Looking for magics...\n");

    // generates random magic number candidates and tests them
    for (int i = 0; i < 1000000; i++)
    {
        for (int sq = 0; sq < 64; sq++)
        {
            U64 toTest = randomU64() & randomU64() & randomU64();

            if (!bishopMagics[sq])
            {
                testMagicNumber(sq, toTest, 1);
            }

            if (!rookMagics[sq])
            {
                testMagicNumber(sq, toTest, 0);
            }
        }
    }

    // show current size of table and all of the rook magics
    displayTableStats();

    return 0;
}

int testMagicNumber(int s, U64 number, int isBishop)
{
    // zero out the testAttacks array
    // Courtesy of
    // https://stackoverflow.com/questions/9146395/reset-c-int-array-to-zero-the-fastest-way
    memset(testAttacks, 0, sizeof(testAttacks));

    U64 maxDefenders = isBishop ? bishopMasks[s] : rookMasks[s];

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
    for (int c = 0; c < (1 << bits); c++)
    {
        // generate test mask given bit indexes
        U64 testMask = 0ULL;
        for (int b = 0; b < bits; b++)
        {
            testMask |= (1ULL << bitIndexes[b]) * ((c & (1ULL << b)) > 0);
        }

        // use the testMask to test the magic number
        int testIndex = (int)((testMask * number) >> (64 - bits));
        U64 attacks = isBishop ? genBishopAttacks(s, testMask) : genRookAttacks(s, testMask);

        //printf("testIndex: %d\n", testIndex);

        // if this won't make the table smaller, don't bother
        // or if it even exceeds the max index
        if ((isBishop ? bishopSmallest[s] : rookSmallest[s]) < testIndex || testIndex >= MAX_BOARD_INDEX)
        {
            success = 0;
            break;
        }

        // is this a unique index OR a constructive collision?
        U64 stored = testAttacks[testIndex];
        if (!stored || stored == attacks)
        {
            testAttacks[testIndex] = attacks;
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
        printf("New ");
        if (isBishop)
        {
            bishopMagics[s] = number;
            bishopSmallest[s] = smallest;
            printf("bishop ");
        }
        else
        {
            rookMagics[s] = number;
            rookSmallest[s] = smallest;
            printf("rook ");
        }
        printf("magic number for square %d: %llu largest index is %d\n", s, number, smallest);
    }

    return success;
}

void displayTableStats(void)
{
    // get the smallest board index necessary
    int smallestR = MAX_BOARD_INDEX;
    int smallestB = MAX_BOARD_INDEX;
    int validR = 0;
    int validB = 0;
    for (int s = 0; s < 64; s++)
    {
        if (rookSmallest[s] < smallestR)
        {
            smallestR = rookSmallest[s];
        }
        if (bishopSmallest[s] < smallestB)
        {
            smallestB = bishopSmallest[s];
        }
        validR += rookMagics[s] > 0;
        validB += bishopMagics[s] > 0;
    }
    if (validR == 64)
    {
        printf("Rook table size:\n");
        printf("%d bytes\n", (smallestR + 1) * 64 * sizeof(U64));
    }
    else
    {
        printf("Rook table is still missing %d square(s)\n", 64 - validR);
    }
    if (validB == 64)
    {
        printf("Bishop table size:\n");
        printf("%d bytes\n", (smallestB + 1) * 64 * sizeof(U64));
    }
    else
    {
        printf("Bishop table is still missing %d square(s)\n", 64 - validB);
    }

    // now print all magics
    printf("U64 rookMagics[64] =\n");
    printf("{\n");
    for (int i = 0; i < 64; i++)
    {
        printf("\t%lluULL,\n", rookMagics[i]);
    }
    printf("};\n");

    printf("U64 bishopMagics[64] =\n");
    printf("{\n");
    for (int i = 0; i < 64; i++)
    {
        printf("\t%lluULL,\n", bishopMagics[i]);
    }
    printf("};\n");
}

void genRookMasks(void)
{
    
    for (int s = 0; s < 64; s++)
    {
        // rank and file of square
        int rank = get_rank(s);
        int file = get_file(s);

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
    }
}

void genBishopMasks(void)
{
    
    for (int s = 0; s < 64; s++)
    {
        // rank and file of square
        int rank = get_rank(s);
        int file = get_file(s);

        // create relevant occupancy mask for a rook at this square (in each direction excluding last square)
        U64 mask = 0ULL;
        for (int f = file, r = rank; f > 0 && r > 0; f--, r--)
        {
            mask |= 1ULL << (f + r * 8);
        }

        for (int f = file, r = rank; f < 7 && r < 7; f++, r++)
        {
            mask |= 1ULL << (f + r * 8);
        }

        for (int r = rank, f = file; r > 0 && f < 7; r--, f++)
        {
            mask |= 1ULL << (f + r * 8);
        }

        for (int r = rank, f = file; r < 7 && f > 0; r++, f--)
        {
            mask |= 1ULL << (f + r * 8);
        }

        // rook cannot capture where it currently is
        mask ^= 1ULL << s;

        // generate attacks and send them to the lookup table
        bishopMasks[s] = mask;
    }
}
