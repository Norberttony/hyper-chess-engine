#ifndef PERFT_HEADER
#define PERFT_HEADER

#include "move.h"

// general debug utilities that tend to be focused on counting the number of moves from the current
// position to a certain depth.

struct MoveCounter
{
    int moves;
    int captureMoves;
    int pieceCaptures;
    int checkmates;
};

// returns the number of moves at the given depth
struct MoveCounter countMoves(int depth);
struct MoveCounter divide(int depth);

// this is insanely temporary. for testing/debugging purposes.
int isMoveLegal(Move);

int countCaptures(Move);

#endif