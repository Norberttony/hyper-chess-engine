#ifndef PERFT_HEADER
#define PERFT_HEADER

#include "bitboard-utility.h"
#include "move.h"

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

Move chooseMove(int startSq, int endSq);

// chooses a move without checking if it is legal or not.
Move chooseMoveBlind(int startSq, int endSq);

int isAttackingKing(int toPlay, int notToPlay);

// just played a move. is it checkmate?
int isCheckmate(void);

// returns the piece type if the square is controlled by the side to play, and 0 if it is not.
int isSquareControlled(int stp, int sq, int pieceType);

int isSquareControlledByStraddler(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByRetractor(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledBySpringer(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByCoordinator(int stp, int sq, U64 notImmInfl, U64 totalBoard, int pieceType);
int isSquareControlledByKing(int stp, int sq, U64 notImmInfl, U64 totalBoard);

#endif
