#ifndef MAGIC_BITBOARDS
#define MAGIC_BITBOARDS

#include "bitboard-utility.h"

extern U64 rookMagics[64];
extern U64 rookMasks[64];
extern int rookMaskBitCount[64];
extern U64 rookAttacks[102400];
extern U64 *rookRefs[64];

extern U64 bishopMagics[64];
extern U64 bishopMasks[64];
extern int bishopMaskBitCount[64];
extern U64 bishopAttacks[5248];
extern U64* bishopRefs[64];

#define get_rook_attacks(sq, blockers) *(rookRefs[sq] + (((rookMasks[sq] & blockers) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])))
#define get_bishop_attacks(sq, blockers) *(bishopRefs[sq] + (((bishopMasks[sq] & blockers) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])))
#define get_queen_attacks(sq, blockers) (get_rook_attacks(sq, blockers) | get_bishop_attacks(sq, blockers))


void initMagicBitboards(int isBishop);

// assumes that sq is not set to 1 in blockers
U64 genRookAttacks(int sq, U64 blockers);
U64 genBishopAttacks(int sq, U64 blockers);

// calculates a to the power of b
int intPow(int a, int b);

#endif
