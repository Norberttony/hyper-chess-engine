#ifndef MOVE_HEADER
#define MOVE_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"

// defines the max depth that the engine can calculate to.
#define MAX_DEPTH 128

// defines the maximum possible number of moves in any attainable position
#define MAX_MOVES 256
#define MAX_CAPTURES 90

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

extern const int move_cham_c_mask;  // capture mask
extern const int move_cham_u_mask;  // straddler up
extern const int move_cham_l_mask;  // left
extern const int move_cham_r_mask;  // right
extern const int move_cham_d_mask;  // down
extern const int move_cham_d1_mask; // death square 1
extern const int move_cham_d2_mask; // death square 2
extern const int move_cham_q_mask;  // retractor
extern const int move_cham_n_mask;  // springer

extern const int move_captMask; // all capture bits

extern const char* squareNames[];

extern const U64 straddlerBounds[];

// pass in pointer to array, which is then populated with the pseudo-legal moves.
// returns the number of moves
int generateMoves(Move*, int capturesOnly);

int generateStraddlerMoves(int sq, U64 moves, Move* movelist);

int generateImmobilizerMoves(int sq, U64 moves, Move* movelist);

int generateCoordinatorMoves(int sq, U64 moves, Move* movelist);
int generateCoordinatorCaptures(int sq, U64 moves, Move* movelist);

int generateKingMoves(int sq, U64 moves, Move* movelist, int capturesOnly);

// assumes that the given moves do not capture any pieces
int generateSpringerMoves(int sq, U64 moves, Move* movelist);

int generateSpringerCaptures(int sq, U64 moves, Move* movelist);

// assumes that the given moves do not capture any pieces
int generateRetractorMoves(int sq, U64 moves, Move* movelist);

int generateRetractorCaptures(int sq, U64 moves, Move* movelist, int capturesOnly);

int generateChameleonRookMoves(int sq, U64 moves, Move* movelist, U64, U64, U64, U64, int capturesOnly);
int generateChameleonBishopMoves(int sq, U64 moves, Move* movelist, int capturesOnly);
int generateChameleonSpringerCaptures(int sq, U64 moves, Move* movelist);

void printMove(Move);
void prettyPrintMove(Move);

#endif