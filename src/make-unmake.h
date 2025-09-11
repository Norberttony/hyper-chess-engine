#ifndef MAKE_UNMAKE_HEADER
#define MAKE_UNMAKE_HEADER

#include "evaluate-defines.h"
#include "move.h"


void makeMove(Move m);
void unmakeMove(Move m);

// used to pass (or "unpass") a turn
void makeNullMove(void);

#endif
