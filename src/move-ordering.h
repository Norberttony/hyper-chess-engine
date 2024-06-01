#ifndef MOVE_ORDERING_HEADER
#define MOVE_ORDERING_HEADER

#include "move.h"

extern Move orderFirst; // the move to order first

// meant to be used with quicksort.
// assuming a and b are both Moves, this function returns > 0 if a is more valuable, < 0 if b is
// more valuable, and 0 if both moves are equally valuable.
int compareMoves(const void *a, const void *b);

#endif