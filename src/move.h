#ifndef MOVE_HEADER
#define MOVE_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"

// 32-bit moves
// LSB
// - first 3 bits for what piece is moving
// - next 6 from bits
// - next 6 to bits
// - next 13 capture bits, meaning changes based on piece
// - - for the straddler, U L R D (directions) piece captures
// - - for the immobilizer, always empty
// - - for the coordinator, two death squares
// - - for the retractor, just the piece it captured
// - - for the springer, just the piece it captured
// - - for the chameleon, 8 bits ULRDTBRN (up left right down top bottom retractor springer)
// - - for the king, displacement, two death squares, four bits for which chameleon it coordinated with (if any)
typedef uint32_t Move;

extern const int move_typeMask; // piece performing move
extern const int move_fromMask; // from
extern const int move_toMask;   // to
extern const int move_c1Mask;   // tends to be first capture
extern const int move_c2Mask;   // tends to be second capture
extern const int move_c3Mask;   // tends to be third capture
extern const int move_c4Mask;   // tends to be fourth capture
extern const int move_kingcmask;
extern const int move_kingc1mask;
extern const int move_kingc2mask;
extern const int move_kingc3mask;
extern const int move_kingc4mask;
extern const int move_captMask; // all capture bits

struct MoveList
{
    Move list[300];
    int size;
};

extern const char* squareNames[];

struct MoveList* generateMoves();
void generateStraddlerMoves(int sq, U64 moves, struct MoveList*);
void generateImmobilizerMoves(int sq, U64 moves, struct MoveList* movelist);
void generateCoordinatorMoves(int sq, U64 moves, struct MoveList* movelist);
void generateKingMoves(int sq, U64 moves, struct MoveList* movelist);

// assumes that the given moves do not capture any pieces
void generateSpringerMoves(int sq, U64 moves, struct MoveList* movelist);

void generateSpringerCaptures(int sq, U64 moves, struct MoveList* movelist);

// assumes that the given moves do not capture any pieces
void generateRetractorMoves(int sq, U64 moves, struct MoveList* movelist);

void generateRetractorCaptures(int sq, U64 moves, struct MoveList* movelist);

void prettyPrintMove(Move);

void makeMove(Move);
void unmakeMove(Move);

#endif