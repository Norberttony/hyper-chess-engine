#ifndef THINK_HEADER
#define THINK_HEADER

#include "../movegen/move.h"
#include "search-defines.h"

#define USE_TRANSPOSITION_TABLE

// the amount of moves to reduce
#define NULL_MOVE_R 3

// performs a min-max alpha-beta search from the current position to the given depth.
// uses transposition table (if enabled) and performs a quiescent search at the fringe nodes.
// returns the evaluation of the position.
int think(int depth, int alpha, int beta, SearchResults* res, SearchFlags flags);

void startThink(SearchParams* s, SearchResults* res);

// a quiescent search, this function only performs min-max alphabeta pruning on sequences of
// captures
int thinkCaptures(int alpha, int beta, SearchResults* res, int accessTT);

// forward declaration from uci.h
void readInput(void);

#endif
