
#include "p-board.h"

// each ith board in the array indicates that >= i straddlers can move to that square.
U64 pControl[2][4];


void printStraddlerMoveBoard(int color)
{
    if (color == white)
        printf("White's ");
    else
        printf("Black's ");
    puts("Straddler Move Board");
    for (int r = 0; r < 8; r++)
    {
        printf(" %d  ", 8 - r);
        for (int f = 0; f < 8; f++)
        {
            int sqIndex = r * 8 + f;

            // gets number of times the square is controlled
            int controls = 0;
            for (int i = 0; i < 4; i++)
            {
                int val = (pControl[color == black][i] & (1ULL << sqIndex)) > 0;
                controls += val;
            }

            printf("%d ", controls);
        }
        printf("\n");
    }
    
    puts("    a b c d e f g h");
}

// removes a single point of control
void erase(U64 board, int color)
{
    for (int i = 3; i >= 0; i--)
    {
        U64 overlap = pControl[color == black][i] & board;
        pControl[color == black][i] &= ~board;
        board &= ~overlap;
    }
}

// adds a single point of control
void place(U64 board, int color)
{
    for (int i = 0; i < 4; i++)
    {
        U64 overlap = pControl[color == black][i] & board;
        pControl[color == black][i] |= board;
        board &= overlap;
    }
}

void pickupPiece(int sq, int colorType)
{
    // if sq is negative it should be set to 0 by isValid
    int isValid = !(colorType & P_BOARD_INVALID_BIT);
    if (!isValid)
        return;

    sq *= isValid;

    U64 totalBoard = position[white] | position[black];
    U64 straddlerBoard = position[white + straddler] | position[black + straddler];
    U64 pieceRookAttacks = get_rook_attacks(sq, totalBoard);
    U64 straddlerImpact = pieceRookAttacks & straddlerBoard;
    pieceRookAttacks &= ~totalBoard;

    // individually separate each potentially affected straddler.
    U64 left  = straddlerImpact & leftBoards[sq];
    U64 above = straddlerImpact & aboveBoards[sq];
    U64 below = straddlerImpact & belowBoards[sq];
    U64 right = straddlerImpact & rightBoards[sq];

    // determine what control the straddlers gain back
    U64 addLeft  = pieceRookAttacks & rightBoards[sq] | 1ULL << sq;
    U64 addAbove = pieceRookAttacks & belowBoards[sq] | 1ULL << sq;
    U64 addBelow = pieceRookAttacks & aboveBoards[sq] | 1ULL << sq;
    U64 addRight = pieceRookAttacks &  leftBoards[sq] | 1ULL << sq;

    // if invalid (a piece wasn't actually picked up), ignore
    pieceRookAttacks  *= isValid;
    addLeft  *= isValid;
    addAbove *= isValid;
    addBelow *= isValid;
    addRight *= isValid;

    // place down the control on the correct board
    place(addLeft  * (left > 0),  8 * ((left  & position[black + straddler]) > 0));
    place(addAbove * (above > 0), 8 * ((above & position[black + straddler]) > 0));
    place(addBelow * (below > 0), 8 * ((below & position[black + straddler]) > 0));
    place(addRight * (right > 0), 8 * ((right & position[black + straddler]) > 0));

    // if the piece is a straddler, its influence should be removed
    // this only works because straddler = 1
    erase(pieceRookAttacks * ((colorType & 0b111) == straddler), colorType & 0b1000);
}

void placePiece(int sq, int colorType)
{
    // if sq is negative it should be set to 0 by isValid
    int isValid = !(colorType & P_BOARD_INVALID_BIT);
    if (!isValid)
        return;

    sq *= isValid;

    U64 totalBoard = position[white] | position[black];
    U64 straddlerBoard = position[white + straddler] | position[black + straddler];
    U64 pieceRookAttacks = get_rook_attacks(sq , totalBoard);
    U64 straddlerImpact = pieceRookAttacks & straddlerBoard;
    pieceRookAttacks &= ~totalBoard;

    // individually separate each potentially affected straddler.
    U64 left  = straddlerImpact & leftBoards[sq];
    U64 above = straddlerImpact & aboveBoards[sq];
    U64 below = straddlerImpact & belowBoards[sq];
    U64 right = straddlerImpact & rightBoards[sq];

    // determine what control the straddlers lose
    U64 addLeft  = pieceRookAttacks & rightBoards[sq] | 1ULL << sq;
    U64 addAbove = pieceRookAttacks & belowBoards[sq] | 1ULL << sq;
    U64 addBelow = pieceRookAttacks & aboveBoards[sq] | 1ULL << sq;
    U64 addRight = pieceRookAttacks &  leftBoards[sq] | 1ULL << sq;

    // if invalid (a piece wasn't actually picked up), ignore
    pieceRookAttacks  *= isValid;
    addLeft  *= isValid;
    addAbove *= isValid;
    addBelow *= isValid;
    addRight *= isValid;

    // erase the influence on the correct board
    erase(addLeft  * (left > 0),  8 * ((left  & position[black + straddler]) > 0));
    erase(addAbove * (above > 0), 8 * ((above & position[black + straddler]) > 0));
    erase(addBelow * (below > 0), 8 * ((below & position[black + straddler]) > 0));
    erase(addRight * (right > 0), 8 * ((right & position[black + straddler]) > 0));

    // if the piece is a straddler, its influence should be added back
    // this only works because straddler = 1
    place(pieceRookAttacks * ((colorType & 0b111) == straddler), colorType & 0b1000);
}
