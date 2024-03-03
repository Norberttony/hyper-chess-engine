#ifndef LOOK_UP_TABLES_HEADER
#define LOOK_UP_TABLES_HEADER

#include <stdlib.h>

#include "bitboard-utility.h"

extern U64 kingMoves[64];
extern U64 deathSquares[64][64];

// where the springer captured given where it started and where it moved to
extern U64 springerCaptures[64][64];

// where the springer has to leap to to capture enemy
// springerLeaps[springerSq][enemySq]
extern U64 springerLeaps[64][64];


// populates ranks and files arrays (bitboards set to 1 if on either rank/file)
void populateRanksAndFiles();

// sq1 and sq2 are locations of king/coordinator, doesn't matter which is which.
U64 genDeathSquares(int sq1, int sq2);

// populates deathSquares lookup table
void populateDeathSquares();

// generates moves for a king
U64 genKingMoves(int sqIndex);

// populates king move lookup table
void populateKingMoves();

// generates the valid leap given a springer and an enemy piece (springerLeap)
U64 genSpringerLeap(int springerSq, int enemySq);

// populates springer leap lookup table
void populateSpringerLeaps();

// generates valid capture given a springer and an enemy piece (springerCapture)
U64 genSpringerCapture(int startSq, int endSq);

// populates springer capture lookup table
void populateSpringerCaptures();

#endif