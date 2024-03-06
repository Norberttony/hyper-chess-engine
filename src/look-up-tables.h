#ifndef LOOK_UP_TABLES_HEADER
#define LOOK_UP_TABLES_HEADER

#include <stdlib.h>

#include "bitboard-utility.h"

extern U64 ranks[8];
extern U64 files[8];

extern U64 sqRanks[64];
extern U64 sqFiles[64];

extern U64 kingMoves[64];
extern U64 deathSquares[64][64][2];

// where the springer captured given where it started and where it moved to
extern U64 springerCaptures[64][64];

// where the springer has to leap to capture enemy
// springerLeaps[springerSq][enemySq]
extern U64 springerLeaps[64][64];

// where the retractor has to move to capture an enemy piece
// retractorCaptures[retractorSq][enemySq]
extern U64 retractorCaptures[64][64];


// populates ranks and files arrays (bitboards set to 1 if on either rank/file)
void populateRanksAndFiles();

// sq1 and sq2 are locations of king/coordinator, doesn't matter which is which.
U64 genDeathSquares1(int sq1, int sq2);
U64 genDeathSquares2(int sq1, int sq2);

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

// generates valid capture given a retractor and an enemy piece (retractorCapture)
U64 genRetractorCapture(int startSq, int enemySq);

// populates retractor capture lookup table
void populateRetractorCaptures();

#endif