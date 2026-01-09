#ifndef EVALUATE_DEFINES_HEADER
#define EVALUATE_DEFINES_HEADER

#include "../movegen/position-defines.h"

extern const int immBonus[];
extern const int immLoSPen[];
extern const int immDistPenalties[8];
extern const int pieceValues[];
extern int pieceSquareTables[8][64];

void initPSQT(void);

// returns value from piece square table for a given piece
// since the startpos 
static inline int PSQT(int piece, int col, int sq)
{
    switch(piece)
    {
        case _:
        case straddler:
        case retractor:
        case springer:
        case chameleon:
        case king:
            return pieceSquareTables[piece][col == white ? sq : reflectSq(sq)];
        case coordinator:
        case immobilizer:
            return pieceSquareTables[piece][col == white ? sq : flipSq(sq)];
    }
    return 0;
}

#endif
