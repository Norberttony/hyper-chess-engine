#ifndef PERFT_HEADER
#define PERFT_HEADER

#include "defines.h"
#include "move.h"
#include "make-unmake.h"

// general debug utilities that tend to be focused on counting the number of moves from the current
// position to a certain depth.

typedef struct MoveCounter
{
    U64 moves;
    U64 captureMoves;
    U64 pieceCaptures;
    U64 checkmates;
} MoveCounter;

// returns the number of moves at the given depth
MoveCounter countMoves(int depth, Move prevMove);
MoveCounter divide(int depth, Move prevMove);

// this is insanely temporary. for testing/debugging purposes.
int isMoveLegal(Move move);

int countCaptures(Move move);

int chooseMove(int startSq, int endSq);

// chooses a move without checking if it is legal or not.
int chooseMoveBlind(int startSq, int endSq);

// incomplete, only considers chameleon, king, and coordinator checks (excluding king walking up to king)
// is the side to play attacking the king?
int isAttackingKing(void);

// just played a move. is it checkmate?
int isCheckmate(void);

// returns the piece type if the square (sq) is controlled by side to play (stp)
// otherwise returns 0.
int isSquareControlled(int stp, int sq, int pieceType);

// A "isSquareControlledByChameleon" routine is not included because it is handled by each of these
// functions separately when setting inclCham to 1. inclCham determines if to include chameleons
// into the detecting-square-control. For instance, chameleons do not control ANY squares against
// other chameleons (because they can't capture each other).
int isSquareControlledByStraddler(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByRetractor(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledBySpringer(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByCoordinator(int stp, int sq, U64 notImmInfl, U64 totalBoard, int pieceType);
int isSquareControlledByKing(int stp, int sq, U64 notImmInfl, U64 totalBoard);

#endif
