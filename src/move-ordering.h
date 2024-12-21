#ifndef MOVE_ORDERING_HEADER
#define MOVE_ORDERING_HEADER

#include "move.h"
#include "evaluate.h"


#define MAX_HISTORY 10000


extern Move orderFirst; // the move to order first

extern Move killerMoves[MAX_DEPTH][2];
extern int historyValues[2][64][64];


void orderMoves(Move* moves, int count, int depth);
void addKillerMove(Move m, int depth);
void updateHistory(int from, int to, int bonus);

#endif