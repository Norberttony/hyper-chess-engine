#ifndef BITBOARD_UTILITY_HEADER
#define BITBOARD_UTILITY_HEADER

#include <stdio.h>
#include <stdlib.h>

// definition of a bitboard
typedef unsigned long long U64;

// macros
#define set_bit(board, sq) (board) | (1ULL << sq)
#define pop_lsb(board) DebruijnIndexes64[(((board) & -(board)) * DebruinSequence64) >> 58]
#define remove_lsb(board) ((board - 1) & board)


extern const U64 DebruinSequence64;

extern const int DebruijnIndexes64[];

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

U64 randomU64();

#endif