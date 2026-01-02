
#include "evaluate-defines.h"

// immobilizer receives a bonus for the pieces it immobilizes.
// some pieces don't lose their effectiveness (straddlers still teamwork, chameleon's primary uses
// are immobilizing enemy immobilizer and/or dealing with coordinator, etc.)
const int immBonus[] =
{
    0,          // empty
    20,         // straddler
    80,         // retractor
    100,        // springer
    200,        // coordinator
    250,        // immobilizer
    125,        // chameleon
    400         // king
};

// Penalties based on how many open lines of sight are against the immobilizer.
const int immLoSPen[] =
{
    0, 60, 70, 80, 90
};

// the penalty to apply based on how far (in ranks) the immobilizer is from the home rank.
const int immDistPenalties[8] =
{
    0, 5, 10, 30, 50, 70, 70, 70
};

// the flat bonus for having a piece on the board.
const int pieceValues[] = 
{
    0,          // empty
    100,        // straddler
    300,        // retractor
    400,        // springer
    1000,       // coordinator
    1300,       // immobilizer
    500,        // chameleon
    0           // king (priceless)
};

// when considering squares for black, the board is flipped according to the piece's symmetry:
// Straddlers, retractors, kings, springers, and chameleons have vertical symmetry.
// Coordinators and immobilizers have symmetry across y = x, or "counter symmetry."
int pieceSquareTables[8][64] =
{
    { 0 },
    // for straddlers
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         3,  3,  3,  3,  3,  3,  3,  3,
         3,  3,  3,  3,  3,  3,  3,  3,
         3,  3,  3,  5,  5,  3,  3,  3,
         5,  3,  3,  5,  5,  3,  3,  5,
         0,  0,  0,  0,  0,  0,  0,  0,
        -3, -3, -3, -3, -3, -3, -3, -3,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    // for retractor
    {
        -10, -5, -5, -5, -5, -5, -5,-10,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  5, 10, 10, 10, 10,  5, -5,
         -5,  5, 10, 10, 10, 10,  5, -5, // -5 for edges and -10 for corners
         -5,  0, -5, -5, -5, -5,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
        -10, -5, -5, -5, -5, -5, -5,-10
    },
    // for springers
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         5,  5,  5,  0,  0,  5,  5,  5,
         5, 10,  5,  0,  0,  5, 10,  5,
         0,  0,  0,  0,  0,  0,  0,  0,
        -5, -5, -5, -5, -5, -5, -5, -5
    },
    // for coordinator
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
        -5, -5, -5, -5, -5, -5, -5, -5,
        -5, -5, -5, -5, -5, -5, -5, -5
    },
    // for immobilizer
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,  10,  10,   0,   0, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    },
    // for chameleon
    {
         10,  10,  10,  10,  10,  10,  10,  10,
          5,   5,   5,   5,   5,   5,   5,   5,
          5,   5,   5,   5,   5,   5,   5,   5,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
         -5,  -5,  -5,   3,   3,   3,  -5,  -5,
         -5,  -5,  -5,   3,   3,   3,  -5,  -5
    },
    // for king
    {
        -20, -20, -20, -20, -20, -20, -20, -20,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -10, -10, -10, -10, -10, -10, -10, -10,
         -5, -10, -10, -10, -10, -10, -10,  -5,
         -5,   5,   0,   0,   0,   0,   5,  -5
    },
};

void initPSQT(void)
{
    // iterate through all of the pieces
    for (int i = 1; i <= 7; i++)
    {
        for (int s = 0; s < 64; s++)
        {
            pieceSquareTables[i][s] += pieceValues[i];
        }
    }
}
