#ifndef MOVE_ORDERING_HEADER
#define MOVE_ORDERING_HEADER

#include "perft.h"
#include "move.h"
#include "evaluate.h"


#define MAX_HISTORY 10000
#define CONT_HISTORY_PLY 1

#define killer_move(depth, idx) killerMoves[depth][idx]


extern Move orderFirst; // the move to order first

extern Move killerMoves[MAX_DEPTH][2];
extern int historyValues[2][64][64];
extern int continuationHistory[CONT_HISTORY_PLY][7][64][7][64];


void orderMoves(Move* moves, int count, int height);
void orderCapts(Move* moves, int count);
void addKillerMove(Move m, int depth);
void updateHistory(Move m, int bonus);

#endif
