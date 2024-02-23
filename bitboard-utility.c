
#include "bitboard-utility.h"

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
