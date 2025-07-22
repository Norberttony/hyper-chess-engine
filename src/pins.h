#ifndef PINS_HEADER
#define PINS_HEADER

#include "defines.h"
#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"


// Pins are incredibly difficult to manage in Hyper Chess, so to simplify the problem, pins.c and
// pins.h will generate pins only for quiet moves. This approach was chosen partly because captures
// in this game happen very infrequently (on average ~1 capture per node over thousands of games).


extern U64 g_pinMasks[64];
extern U64 g_pinned;
extern U64 g_checkMask;
extern U64 g_immCheckMask;

// forward declaration from move.h
extern const U64 straddlerBounds[];


// initializes data necessary for pins, should be run once before doing any searching.
void initPins(void);

// generates all of the pin masks, pinned pieces, and the check mask.
// run this before generating moves to get all of the pins for the current position.
void generatePins(void);

void debugPrintPins(void);

#endif
