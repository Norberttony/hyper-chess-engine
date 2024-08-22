
#include "pins.h"

U64 pinMasks[7];

U64 getPinMask(int pieceType, int pieceSq)
{

    // all piece occupancy
    U64 totalBoard = position[white] | position[black];
    U64 myBoard = position[toPlay];
    U64 enemyBoard = position[notToPlay];

    // get squares that enemy immobilizer is not influencing
    U64 enemImm = position[notToPlay + immobilizer];
    U64 notImmInfl = ~(kingMoves[pop_lsb(enemImm)] * (enemImm > 0));

    // get the king we're trying to target
    U64 targetKing = position[toPlay + king];
    int targetKingSq = pop_lsb(targetKing);

    // currently everything can move everywhere
    memset(pinMasks, -1, sizeof(pinMasks));

    // ignore friendly pieces, get all enemy pieces that xray the king via rook/bishop moves
    U64 rookXRay = get_rook_attacks(targetKingSq, enemyBoard) & enemyBoard;
    U64 bishopXRay = get_bishop_attacks(targetKingSq, enemyBoard) & enemyBoard;

    // interpret each bit...
    while (rookXRay)
    {
        // sq of piece that is potentially pinning another piece
        int sq = pop_lsb(rookXRay);
        U64 lineOfAttack = rookXRay & get_rook_attacks(sq, targetKing); // will not include pinner
        U64 attacked = lineOfAttack & position[toPlay];
        int pinnedSq = pop_lsb(attacked); // location of pinned piece
        
        // ensure validity (one piece in line of attack)
        U64 validity = -1 * (remove_lsb(attacked) > 0);

        switch(pieceList[sq])
        {
            case springer:
                // does not consider immobilizer's case :(
                // does not handle situations where there are multiple pieces of the same type :(
                pinMasks[pieceList[pinnedSq] - 1] &= (validity * (springerLeaps[sq][targetKingSq] > 0)) | lineOfAttack;
                break;
        }
    }

    // === RETRACTOR === //


    // === SPRINGER === //
    U64 springerBoard = position[notToPlay + springer] & notImmInfl;
    {
        int sq = pop_lsb(springerBoard);


        springerBoard &= springerBoard - 1;
    }

    // === STRADDLER === //

    // === COORDINATOR === //

    // === KING === //
    

    // === CHAMELEON === //


}
