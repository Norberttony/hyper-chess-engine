#ifndef P_BOARD
#define P_BOARD

#include "defines.h"
#include "magic-bitboards.h"
#include "move.h"

#define P_BOARD_INVALID_BIT 16

extern U64 pControl[2][4];


void printStraddlerMoveBoard(int color);

void erase(U64 board, int color);
void place(U64 board, int color);

void pickupPiece(int sq, int colorType);
void placePiece(int sq, int colorType);

#endif