#ifndef MAGIC_BITBOARDS
#define MAGIC_BITBOARDS

#include "bitboard-utility.h"

extern U64 rookMagics[64];
extern U64 rookMasks[64];
extern int rookMaskBitCount[64];
extern U64 rookAttacks[64][4096];

extern U64 bishopMagics[64];
extern U64 bishopMasks[64];
extern int bishopMaskBitCount[64];
extern U64 bishopAttacks[64][512];

#define get_rook_attacks(sq, blockers) (rookAttacks[sq][((rookMasks[sq] & blockers) * rookMagics[sq]) >> (64 - rookMaskBitCount[sq])])
#define get_bishop_attacks(sq, blockers) bishopAttacks[sq][((bishopMasks[sq] & totalBoard) * bishopMagics[sq]) >> (64 - bishopMaskBitCount[sq])]


void initMagicBitboards(int isBishop);

// assumes that sq is not set to 1 in blockers
U64 genRookAttacks(int sq, U64 blockers);
U64 genBishopAttacks(int sq, U64 blockers);

#endif