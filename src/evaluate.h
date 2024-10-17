#ifndef EVALUATE_HEADER
#define EVALUATE_HEADER

#include "defines.h"
#include "look-up-tables.h"
#include "magic-bitboards.h"
#include "move.h"


extern const int pieceValues[];

extern const int pieceSquareTables[7][64];


// greedy evaluation that counts material based on piece values
int evaluate(void);

// determines the material value of all the pieces captured.
int moveCaptureValue(Move m);

// parameters are stc: side to capture and sq: square to capture
// returns a penalty based on how fast the square is capturable.
int kingCoordCaptPen(int stc, int sq);

#endif