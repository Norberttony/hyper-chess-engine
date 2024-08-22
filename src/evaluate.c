
#include "evaluate.h"

const int pieceValues[] = 
{
    0,          // empty
    100,        // straddler
    300,        // retractor
    400,        // springer
    1100,       // coordinator
    1300,       // immobilizer
    900,        // chameleon
    0           // king (priceless)
};

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
    150,        // immobilizer
    125,        // chameleon
    200         // king
};

// Gives a penalty based on how many lines of sights against an immobilizer are blocked.
// The highest penalty occurs when no lines of sights are blocked.
const int immLoSPen[] =
{
    0, 25, 50, 100, 150
};

const int pieceSquareTables[7][64] =
{
    // for straddlers
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    // for retractor
    {
        -10, -5, -5, -5, -5, -5, -5,-10,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0, 10, 10, 10, 10,  0, -5, // -5 for edges and -10 for corners
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
        -10, -5, -5, -5, -5, -5, -5,-10
    },
    // for springers
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
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
         0,  0,  0,  0,  0,  0,  0,  0,
        -5, -5, -5, -5, -5, -5, -5, -5
    },
    // for immobilizer
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    },
    // for chameleon
    {
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    // for king
    {
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
         30,  20,  20,  20,  20,  20,  20,  30
    },
};

int evaluate()
{
    U64 enemyImmobilizer = position[notToPlay + immobilizer];
    int enemyImmSq = pop_lsb(enemyImmobilizer);
    U64 enemyInfl = (enemyImmobilizer > 0) * kingMoves[enemyImmSq];

    // if enemy immobilizer is immobilized, then it shouldn't be evaluated highly
    int enemyImmImm = (enemyInfl & (position[toPlay + chameleon] | position[toPlay + immobilizer])) > 0;

    U64 myImmobilizer = position[toPlay + immobilizer];
    int myImmSq = pop_lsb(myImmobilizer);
    U64 myInfl = (myImmobilizer > 0) * kingMoves[myImmSq];

    // if friendly immobilizer is immobilized, then it shouldn't be evaluated highly
    int myImmImm = (myInfl & (position[notToPlay + chameleon] | position[notToPlay + immobilizer])) > 0;

    // counts the number of pseudo-legal moves one can perform
    int myMobilityScore = 0;
    int enemyMobilityScore = 0;

    // counts number of pieces EXCLUDING straddlers
    int numPieces = 0;

    // count mobility
    U64 totalBoard = position[toPlay] | position[notToPlay];
    for (int i = 2; i <= 6; i++)
    {
        U64 myBoard = position[toPlay + i] & ~enemyInfl;
        while (myBoard)
        {
            int sq = pop_lsb(myBoard);
            U64 moveBoard = (get_rook_attacks(sq, totalBoard) | get_bishop_attacks(sq, totalBoard)) & ~totalBoard;

            while (moveBoard)
            {
                myMobilityScore++;
                moveBoard &= moveBoard - 1;
            }

            numPieces++;
            myBoard &= myBoard - 1;
        }

        U64 enemyBoard = position[notToPlay + i] & ~myInfl;
        while (enemyBoard)
        {
            int sq = pop_lsb(enemyBoard);
            U64 moveBoard = (get_rook_attacks(sq, totalBoard) | get_bishop_attacks(sq, totalBoard)) & ~totalBoard;

            while (moveBoard)
            {
                enemyMobilityScore++;
                moveBoard &= moveBoard - 1;
            }

            numPieces++;
            enemyBoard &= enemyBoard - 1;
        }
    }
    
    // for when the immobilizer is immobilized by a chameleon
    U64 enemyChamInfl = myImmobilizer * ((myInfl & position[notToPlay + chameleon]) > 0);
    U64 myChamInfl = enemyImmobilizer * ((enemyInfl & position[toPlay + chameleon]) > 0);
    
    int evaluation = 0;
    int perspective = 2 * (toPlay == white) - 1; // am I WTP (1) or BTP (-1)?
    for (int i = 1; i <= 7; i++)
    {
        // count up my material
        U64 myBoard = position[toPlay + i];
        while (myBoard)
        {
            evaluation += pieceValues[i] + pieceSquareTables[i - 1][(toPlay == black) * 63 + perspective * pop_lsb(myBoard)];
            myBoard &= myBoard - 1;
        }

        // count up my immobilized material
        myBoard = position[toPlay + i] & (enemyInfl | enemyChamInfl);
        while (myBoard)
        {
            evaluation -= immBonus[i];
            myBoard &= myBoard - 1;
        }

        // count up opponent's material
        U64 enemyBoard = position[notToPlay + i];
        while (enemyBoard)
        {
            evaluation -= pieceValues[i] + pieceSquareTables[i - 1][(notToPlay == black) * 63 + -perspective * pop_lsb(enemyBoard)];
            enemyBoard &= enemyBoard - 1;
        }

        // count up opponent's immobilized material
        enemyBoard = position[notToPlay + i] & (myInfl | myChamInfl);
        while (enemyBoard)
        {
            evaluation += immBonus[i];
            enemyBoard &= enemyBoard - 1;
        }
    }


    // determines which boards should be used to count as blockers
    U64 enemyDiags = totalBoard;
    U64 enemyLines = totalBoard;

    // determines whether to test left to right (-1) or up to down (1)
    int testUpDn = myImmobilizer & ranks[0] | myImmobilizer & ranks[7];
    int testLtRt = myImmobilizer & files[7] | myImmobilizer & files[0];
    int onlyTest = testUpDn - testLtRt;

    // count lines of sight for the immobilizer
    int myImmLoS = 
        // top left to bottom right diagonal
        (((myImmobilizer << 9 | myImmobilizer >> 9) & enemyDiags) > 0) * (onlyTest == 0) +
        // up to down
        (((myImmobilizer << 8 | myImmobilizer >> 8) & enemyLines) > 0) * (onlyTest >= 0) +
        // top right to bottom left
        (((myImmobilizer << 7 | myImmobilizer >> 7) & enemyDiags) > 0) * (onlyTest == 0) +
        // left to right
        (((myImmobilizer << 1 | myImmobilizer >> 1) & enemyLines) > 0) * (onlyTest <= 0);

    // handle a corner case (well, literally, the immobilizer being in the corner)
    myImmLoS *= !(testUpDn && testLtRt);

    // apply penalty based on the number of available lines of attack
    evaluation -= ((toPlay == white) * 140 + -perspective * 20 * (myImmSq >> 3) + immLoSPen[myImmLoS]) * myImmImm * (myImmobilizer > 0);


    // determines which boards should be used to count as blockers
    U64 myDiags = totalBoard;
    U64 myLines = totalBoard;

    // determines whether to test left to right (-1) or up to down (1)
    testUpDn = enemyImmobilizer & ranks[0] | enemyImmobilizer & ranks[7];
    testLtRt = enemyImmobilizer & files[7] | enemyImmobilizer & files[0];
    onlyTest = testUpDn - testLtRt;

    // count lines of sight for the immobilizer
    int enemyImmLoS = 
        // top left to bottom right diagonal
        (((enemyImmobilizer << 9 | enemyImmobilizer >> 9) & myDiags) > 0) * (onlyTest == 0) +
        // up to down
        (((enemyImmobilizer << 8 | enemyImmobilizer >> 8) & myLines) > 0) * (onlyTest >= 0) +
        // top right to bottom left
        (((enemyImmobilizer << 7 | enemyImmobilizer >> 7) & myDiags) > 0) * (onlyTest == 0) +
        // left to right
        (((enemyImmobilizer << 1 | enemyImmobilizer >> 1) & myLines) > 0) * (onlyTest <= 0);

    // handle a corner case (well, literally, the immobilizer being in the corner)
    enemyImmLoS *= !(testUpDn && testLtRt);

    // apply penalty based on the number of available lines of attack
    evaluation += ((notToPlay == white) * 140 + perspective * 20 * (enemyImmSq >> 3) + immLoSPen[enemyImmLoS]) * enemyImmImm * (enemyImmobilizer > 0);

    // whoever has more material MUST be winning (not necessarily but y'know)
    return evaluation + myMobilityScore - enemyMobilityScore;
}

int moveCaptureValue(Move m)
{
    switch (m & move_typeMask)
    {
        case straddler:
            return
                pieceValues[(m & move_c1Mask) >> 15] +
                pieceValues[(m & move_c2Mask) >> 18] +
                pieceValues[(m & move_c3Mask) >> 21] +
                pieceValues[(m & move_c4Mask) >> 24];
        case retractor:
        case springer:
            return pieceValues[(m & move_c1Mask) >> 15];
        case coordinator:
            return
                pieceValues[(m & move_c1Mask) >> 15] +
                pieceValues[(m & move_c2Mask) >> 18];
        case immobilizer:
            return 0;
        case chameleon:
            return
                pieceValues[(m & move_cham_u_mask) > 0] +
                pieceValues[(m & move_cham_l_mask) > 0] +
                pieceValues[(m & move_cham_r_mask) > 0] +
                pieceValues[(m & move_cham_d_mask) > 0] +

                ((m & (move_cham_d1_mask | move_cham_d2_mask)) > 0) * pieceValues[coordinator] +
                ((m & move_cham_q_mask) > 0) * pieceValues[retractor] +
                ((m & move_cham_n_mask) > 0) * pieceValues[springer];
        case king:
            return
                pieceValues[(m & move_c1Mask) >> 15] +
                pieceValues[(m & move_c2Mask) >> 18] +
                pieceValues[(m & move_c3Mask) >> 21] +
                ((m & move_kingcmask) > 0) * pieceValues[coordinator];
    }
}
