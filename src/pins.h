#ifndef PINS_HEADER
#define PINS_HEADER

#include <string.h>

#include "defines.h"
#include "look-up-tables.h"
#include "magic-bitboards.h"
#include "perft.h"

// pin masks stored for each piece individually.
extern U64 pinMasks[16];

// returns the squares that a piece must intervene/intercept to prevent the enemy capturing the king
U64 getCheckMask();

// returns the necessary captures that must occur in order to prevent the enemy capturing the king
U64 getCheckCaptureMask();

U64 getPinMask(int pieceType, int pieceSq);

#endif