#ifndef MOVE_HEADER
#define MOVE_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"

// defines the max depth that the engine can calculate to.
#define MAX_DEPTH 127

// defines the maximum possible number of moves in any attainable position
#define MAX_MOVES 256
#define MAX_CAPTURES 90

// most moves are structured such that captured pieces are consecutively ordered
#define get_type(move)  (move & 0x7)
#define get_from(move)  ((move & 0x1F8u) >> 3)
#define get_to(move)    ((move & 0x7E00u) >> 9)
#define get_c1(move)    ((move & 0x38000u) >> 15)
#define get_c2(move)    ((move & 0x1C0000u) >> 18)
#define get_c3(move)    ((move & 0xE00000u) >> 21)
#define get_c4(move)    ((move & 0x7000000u) >> 24)

// king uses c1, c2, c3, but may also capture coordinator with chameleons. this identifies which of
// the four squares it could have been (4 bits).
#define get_kb_c(move)  ((move & 0xF000000u) >> 24)
#define get_kb_c1(move) ((move & 0x1000000u) >> 24)
#define get_kb_c2(move) ((move & 0x2000000u) >> 25)
#define get_kb_c3(move) ((move & 0x4000000u) >> 26)
#define get_kb_c4(move) ((move & 0x8000000u) >> 27)

// chameleon uses a single bit to indicate what piece (and sometimes the relative location, like up
// or down for straddlers) was captured
#define get_b_c(move)   ((move & 0x7F8000u) >> 15)
#define get_b_cu(move)  ((move & 0x008000u) >> 15)
#define get_b_cl(move)  ((move & 0x010000u) >> 16)
#define get_b_cr(move)  ((move & 0x020000u) >> 17)
#define get_b_cd(move)  ((move & 0x040000u) >> 18)
#define get_b_cd1(move) ((move & 0x080000u) >> 19)
#define get_b_cd2(move) ((move & 0x100000u) >> 20)
#define get_b_cq(move)  ((move & 0x200000u) >> 21)
#define get_b_cn(move)  ((move & 0x400000u) >> 22)

#define is_move_capt(move) ((move & 0xFFF8000u) != 0)

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