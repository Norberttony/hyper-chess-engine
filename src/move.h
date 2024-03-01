#ifndef MOVE_HEADER
#define MOVE_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "magic-bitboards.h"
#include "defines.h"

// 32-bit moves
// LSB
// - first 3 bits for what piece is moving
// - next 6 from bits
// - next 6 to bits
// - next 12 capture bits, meaning changes based on piece
typedef uint32_t Move;

extern Move moveList[300];
extern int moveListSize;


void generateMoves();
void prettyPrintMove(Move);

#endif