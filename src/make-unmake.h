#ifndef MAKE_UNMAKE_HEADER
#define MAKE_UNMAKE_HEADER

#include "evaluate-defines.h"
#include "move.h"


void makeMove(Move);
void unmakeMove(Move);

// used to pass (or "unpass") a turn
void makeNullMove();

#endif