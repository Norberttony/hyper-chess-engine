#ifndef MOVE_HEADER
#define MOVE_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "magic-bitboards.h"
#include "defines.h"

// 32-bit moves
// LSB
// - first 3 bits for what piece is moving
// - next 6 from bits
// - next 6 to bits
// - next 12 capture bits, meaning changes based on piece
// - - for the straddler, U L R D (directions) piece captures
typedef uint32_t Move;

struct MoveList
{
    Move list[300];
    int size;
};

struct MoveList* generateMoves();
void generateStraddlerMoves(int sq, U64 moves, struct MoveList*);
void prettyPrintMove(Move);

void makeMove(Move);
void unmakeMove(Move);

#endif