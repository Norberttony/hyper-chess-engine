#ifndef PERFT_HEADER
#define PERFT_HEADER

#include "move.h"
#include "make-unmake.h"

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

// returns the piece type if the square is controlled by the side to play, and 0 if it is not.
int isSquareControlled(int stp, int sq, int pieceType);

int isSquareControlledByStraddler(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByRetractor(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledBySpringer(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByCoordinator(int stp, int sq, U64 notImmInfl, U64 totalBoard, int inclCham);
int isSquareControlledByKing(int stp, int sq, U64 notImmInfl, U64 totalBoard);

#endif
