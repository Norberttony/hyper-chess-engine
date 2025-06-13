#ifndef EVALUATE_HEADER
#define EVALUATE_HEADER

#include "evaluate-defines.h"
#include "defines.h"
#include "look-up-tables.h"
#include "magic-bitboards.h"
#include "move.h"


#define imm_dist_penalty(sq) (20 * (sq >> 3))


extern int EVAL_DBG_PRINT;


// greedy evaluation that counts material based on piece values
int evaluate(void);

// determines the material value of all the pieces captured.
int moveCaptureValue(Move m);

#endif
