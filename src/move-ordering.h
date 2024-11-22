#ifndef MOVE_ORDERING_HEADER
#define MOVE_ORDERING_HEADER

#include "move.h"
#include "evaluate.h"


#define MAX_HISTORY 99999999


extern Move orderFirst; // the move to order first

extern Move killerMoves[MAX_DEPTH][2];
extern int historyValues[2][8][64];


void orderMoves(Move* moves, int count, int depth);

#endif