#ifndef LOOK_UP_TABLES_HEADER
#define LOOK_UP_TABLES_HEADER

#include "../movegen/bitboard-utility.h"
#include <stdint.h>

extern U64 ranks[8];
extern U64 files[8];

extern U64 leftBound;
extern U64 rightBound;

extern U64 kingMoves[64];

// where the springer captured given where it started and where it moved to
extern U64 springerCaptures[64][64];

// where the springer has to leap to capture enemy
// springerLeaps[springerSq][enemySq]
extern U64 springerLeaps[64][64];

// where the retractor has to move to capture an enemy piece
// retractorCaptures[retractorSq][enemySq]
extern U64 retractorCaptures[64][64];

// populates ranks and files arrays (bitboards set to 1 if on either rank/file)
void populateRanksAndFiles(void);

// generates moves for a king
U64 genKingMoves(int sqIndex);

// populates king move lookup table
void populateKingMoves(void);

// generates the valid leap given a springer and an enemy piece (springerLeap)
U64 genSpringerLeap(int springerSq, int enemySq);

// populates springer leap lookup table
void populateSpringerLeaps(void);

// generates valid capture given a springer and an enemy piece (springerCapture)
U64 genSpringerCapture(int startSq, int endSq);

// populates springer capture lookup table
void populateSpringerCaptures(void);

// generates valid capture given a retractor and an enemy piece (retractorCapture)
U64 genRetractorCapture(int startSq, int enemySq);

// populates retractor capture lookup table
void populateRetractorCaptures(void);

inline U64 get_death_square_1(int sq1, int sq2)
{
    // get files and ranks of both squares
    int f1 = get_file(sq1);
    int r1 = get_rank(sq1);

    int f2 = get_file(sq2);
    int r2 = get_rank(sq2);

    // use ranks and files bitboard to find intersections
    return (U64)(f1 != f2 && r1 != r2) * (ranks[r1] & files[f2]);
}

inline U64 get_death_square_2(int sq1, int sq2)
{
    // get files and ranks of both squares
    int f1 = get_file(sq1);
    int r1 = get_rank(sq1);

    int f2 = get_file(sq2);
    int r2 = get_rank(sq2);

    // use ranks and files bitboard to find intersections
    return (U64)(f1 != f2 && r1 != r2) * (ranks[r2] & files[f1]);
}

// returns the retractor's capture square given where it's moving from and moving to.
inline U64 get_retractor_capture_sq(int from, int to)
{
    int relDir = from - to;

    // ensure from/to are -1 to 1 file off, and that relDir masks correctly.
    int fileDiff = get_file(from) - get_file(to);
    int isRelDirInRange = relDir >= -9 && relDir <= 9 && fileDiff >= -1 && fileDiff <= 1;
    uint32_t diffMask = (isRelDirInRange * 1ULL) << (isRelDirInRange * (relDir + 9));

    // now check stuff for the capture square
    int sq = from + relDir;
    int isInRange = sq >= 0 && sq < 64 && (460039 & diffMask);
    int isOnLeft = get_file(from) < 4;
    U64 sqBoard = (1ULL * isInRange) << (sq * isInRange);
    sqBoard &= isOnLeft * rightBound + !isOnLeft * leftBound;
    return sqBoard;
}

#endif
