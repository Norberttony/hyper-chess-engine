#ifndef BITBOARD_UTILITY_HEADER
#define BITBOARD_UTILITY_HEADER

#include <stdio.h>
#include <stdlib.h>

// definition of a bitboard
typedef unsigned long long U64;

// macros
#define set_bit(board, sq) (board) | (1ULL << sq)
#define pop_lsb(board) g_DebruijnIndexes64[(((board) & -(board)) * g_DebruinSequence64) >> 58]
#define remove_lsb(board) ((board - 1) & board)
#define get_rank(sq) (sq >> 3)
#define get_file(sq) (sq & 0x7)

extern const U64 g_DebruinSequence64;
extern const int g_DebruijnIndexes64[];

// easier representation of square indices
enum
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

void printBitboard(U64 bitboard);
U64 randomU64(void);

// reflects the square vertically with the board center
static inline int reflectSq(int sq)
{
    return (56 - (sq & 56)) | get_file(sq);
}

// returns the square as if the board was flipped 180 degrees
static inline int flipSq(int sq)
{
    return (56 - (sq & 56)) | (7 - get_file(sq));
}

static inline int countBits(U64 board)
{
    int c = 0;
    while (board)
    {
        c++;
        board &= board - 1;
    }
    return c;
}

#endif
