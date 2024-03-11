
#include "bitboard-utility.h"

const U64 DebruinSequence64 = 0b0000001000101001001101010110010111000111011011110011111101000011ULL;

const int DebruijnIndexes64[] = 
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
            printf("%d ", (bitboard >> sqIndex) & 1);
        }
        printf("\n");
    }
    
    puts("    a b c d e f g h");
    printf("%llu\n\n", bitboard);
}

U64 randomU64()
{
    return ((U64)(rand() & 0xFFFF) << 48) | ((U64)(rand() & 0xFFFF) << 32) | ((U64)(rand() & 0xFFFF) << 16) | (U64)(rand() & 0xFFFF);
}
