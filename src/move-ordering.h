#ifndef MOVE_ORDERING_HEADER
#define MOVE_ORDERING_HEADER

#include "move.h"
#include "evaluate.h"


extern Move orderFirst; // the move to order first

extern Move killerMoves[MAX_DEPTH][2];


void orderMoves(Move* moves, int count, int depth);

#endif