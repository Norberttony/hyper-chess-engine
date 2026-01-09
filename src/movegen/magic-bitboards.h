#ifndef MAGIC_BITBOARDS
#define MAGIC_BITBOARDS

#include "../movegen/bitboard-utility.h"

#define get_rook_attacks(sq, blockers)   *(rookEntries[sq].ref + (((rookEntries[sq].mask & blockers) * rookEntries[sq].magic) >> (64 - rookEntries[sq].maskBitCount)))
#define get_bishop_attacks(sq, blockers) *(bishopEntries[sq].ref + (((bishopEntries[sq].mask & blockers) * bishopEntries[sq].magic) >> (64 - bishopEntries[sq].maskBitCount)))
#define get_queen_attacks(sq, blockers) (get_rook_attacks(sq, blockers) | get_bishop_attacks(sq, blockers))

// to minimize cache misses for fetching attacks on a square, any magic relating to a square has
// been grouped together in a struct
typedef struct MagicSqEntry
{
    U64 magic;
    U64 mask;
    U64* ref;
    int maskBitCount;
} MagicSqEntry;

extern MagicSqEntry rookEntries[64];
extern MagicSqEntry bishopEntries[64];

extern U64 rookAttacks[102400];
extern U64 bishopAttacks[5248];

void initMagicBitboards(int isBishop);

// assumes that sq is not set to 1 in blockers
U64 genRookAttacks(int sq, U64 blockers);
U64 genBishopAttacks(int sq, U64 blockers);

// calculates a to the power of b
int intPow(int a, int b);

#endif
