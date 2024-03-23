
#include "evaluate.h"

const int pieceValues[] = 
{
    0,          // empty
    100,        // straddler
    300,        // retractor
    400,        // springer
    800,        // coordinator
    800,        // immobilizer
    500,        // chameleon
    0           // king (priceless)
};

// if a piece is immobilized, it drops in value.
// note: pieces that immobilize the immobilizer back do not change in value.
const int immobilizedPieceValues[] =
{
    0,          // empty
    90,         // straddler
    200,        // retractor
    300,        // springer
    800,        // coordinator (bonus if corner)
    600,        // immobilizer
    500,        // chameleon
    -250,       // king (bonus if corner) which technically has a value of 0
};

const int pieceSquareTables[7][64] =
{
    // for straddlers
    {
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
         10,  10,   5,   5,   5,   5,  10,  10,
         -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
          8,   8,  10,  10,  10,  10,   8,   8,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    // for retractor
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 10, 10, 10, 10,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0
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
        -80, -80, -80, -80, -80, -80, -80, -80,
        -50, -50, -50, -50, -50, -50, -50, -50,
        -30, -30, -30, -30, -30, -30, -30, -30,
        -20, -20, -20, -20, -20, -20, -20, -20,
        -10, -10, -10, -10, -10, -10, -10, -10,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    // for chameleon
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
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

    int evaluation = 0;
    int perspective = 2 * (toPlay == white) - 1; // am I WTP (1) or BTP (-1)?
    for (int i = 1; i <= 7; i++)
    {
        // count up my material
        U64 myBoard = position[toPlay + i] & ~(enemyInfl);// | (-1 * (i == immobilizer && myImmImm)));// * !enemyImmImm);
        while (myBoard)
        {
            evaluation += pieceValues[i] + pieceSquareTables[i - 1][(toPlay == black) * 63 + perspective * pop_lsb(myBoard)];
            myBoard &= myBoard - 1;
        }

        // count up my immobilized material (only if enemy immobilizer is not immobilized)
        myBoard = position[toPlay + i] & enemyInfl;
        while (myBoard)
        {
            evaluation += immobilizedPieceValues[i] + pieceSquareTables[i - 1][(toPlay == black) * 63 + perspective * pop_lsb(myBoard)];// * !enemyImmImm;
            myBoard &= myBoard - 1;
        }

        // count up opponent's material
        U64 enemyBoard = position[notToPlay + i] & ~(myInfl);// | (-1 * (i == immobilizer && enemyImmImm)));// * !myImmImm);
        while (enemyBoard)
        {
            evaluation -= pieceValues[i] + pieceSquareTables[i - 1][(notToPlay == black) * 63 + -perspective * pop_lsb(enemyBoard)];
            enemyBoard &= enemyBoard - 1;
        }

        // count up opponent's immobilized material (only if my immobilizer is not immobilized)
        enemyBoard = position[notToPlay + i] & myInfl;
        while (enemyBoard)
        {
            evaluation -= immobilizedPieceValues[i] + pieceSquareTables[i - 1][(notToPlay == black) * 63 + -perspective * pop_lsb(enemyBoard)];// * !myImmImm;
            enemyBoard &= enemyBoard - 1;
        }
    }

    int enemKingCornered = (myInfl & position[notToPlay + king]) > 0 && bishopAttacks[myImmSq][0] & position[notToPlay + king];
    int enemCoordCornered = (myInfl & position[notToPlay + coordinator]) > 0 && bishopAttacks[myImmSq][0] & position[notToPlay + coordinator];

    int myKingCornered = (enemyInfl & position[toPlay + king]) > 0 && bishopAttacks[enemyImmSq][0] & position[toPlay + king];
    int myCoordCornered = (enemyInfl & position[toPlay + coordinator]) > 0 && bishopAttacks[enemyImmSq][0] & position[toPlay + coordinator];

    // bonus if king OR coordinator are in an immobilizer's corner
    evaluation += 100 * (enemKingCornered || enemCoordCornered);
    evaluation -= 100 * (myKingCornered || myCoordCornered);

    // whoever has more material MUST be winning (not necessarily but y'know)
    return evaluation;
}