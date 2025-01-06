#ifndef EVALUATE_DEFINES_HEADER
#define EVALUATE_DEFINES_HEADER

#define PSQT(piece, col, sq) pieceSquareTables[piece][((col) == black) * 63 - (2 * ((col) == black) - 1) * (sq)]

extern const int immBonus[];

extern const int immLoSPen[];

extern const int pieceValues[];

extern int pieceSquareTables[8][64];


void initPSQT(void);

#endif