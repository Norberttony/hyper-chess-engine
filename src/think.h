#ifndef THINK_HEADER
#define THINK_HEADER

#include <math.h>
#include "perft.h"
#include "move.h"
#include "move-ordering.h"
#include "transposition-table.h"
#include "evaluate.h"
#include "utils.h"
#include "debug.h"

#define USE_TRANSPOSITION_TABLE

// the amount of moves to reduce
#define NULL_MOVE_R 3


extern SearchParams g_searchParams;


// performs a min-max alpha-beta search from the current position to the given depth.
// uses transposition table (if enabled) and performs a quiescent search at the fringe nodes.
// returns the evaluation of the position.
int think(int depth, int alpha, int beta, SearchFlags flags);

// performs a min-max alpha-beta search up to the given depth and returns the best move.
Move getBestMove(int depth);

// attempts to think for the specified amount of time in ms
Move thinkFor(int ms);

Move startThink(void);

// a quiescent search, this function only performs min-max alphabeta pruning on sequences of
// captures
int thinkCaptures(int alpha, int beta, int accessTT);

// forward declaration from uci.h
void readInput(void);

#endif
