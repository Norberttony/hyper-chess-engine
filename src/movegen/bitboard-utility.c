#include "bitboard-utility.h"
#include <stdio.h>
#include <stdlib.h>

const U64 g_DebruinSequence64 = 0x2293565C76F3F43;
const int g_DebruijnIndexes64[] = 
{
    0, 1, 2, 58, 3, 7, 59, 34, 4, 13, 8, 27, 60, 16, 35, 48, 56,
    5, 11, 14, 9, 20, 22, 28, 61, 24, 17, 40, 30, 36, 43, 49, 63,
    57, 6, 33, 12, 26, 15, 47, 55, 10, 19, 21, 23, 39, 29, 42, 62,
    32, 25, 46, 54, 18, 38, 41, 31, 45, 53, 37, 44, 52, 51, 50
};

void printBitboard(U64 bitboard)
{
    for (int r = 0; r < 8; r++)
    {
        printf(" %d  ", 8 - r);
        for (int f = 0; f < 8; f++)
        {
            int sqIndex = r * 8 + f;
            printf("%llu ", (bitboard >> sqIndex) & 1);
        }
        printf("\n");
    }
    
    puts("    a b c d e f g h");
    printf("%llu\n\n", bitboard);
}

static U64 rand_state = 1;

void my_srand(U64 seed)
{
    rand_state = seed;
}

// Code courtesy of
// https://en.wikipedia.org/wiki/Xorshift#Non-linear_variations
// xorshift64s, variant A_1(12,25,27) with multiplier M_32 from line 3 of table 5
U64 xorshift64star(void) {
    rand_state ^= rand_state >> 12;
    rand_state ^= rand_state << 25;
    rand_state ^= rand_state >> 27;
    return rand_state * 0x2545F4914F6CDD1DULL;
}

U64 randomU64(void)
{
    return xorshift64star();
}
