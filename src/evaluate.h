#ifndef EVALUATE_HEADER
#define EVALUATE_HEADER

#include "defines.h"
#include "look-up-tables.h"
#include "magic-bitboards.h"
#include "move.h"

// greedy evaluation that counts material based on piece values
int evaluate();

// determines the material value of all the pieces captured.
int moveCaptureValue(Move m);

#endif