
#include "evaluate.h"


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

            enemyBoard &= enemyBoard - 1;
        }
    }
    
    // for when the immobilizer is immobilized by a chameleon
    U64 enemyChamInfl = myImmobilizer * ((myInfl & position[notToPlay + chameleon]) > 0);
    U64 myChamInfl = enemyImmobilizer * ((enemyInfl & position[toPlay + chameleon]) > 0);

    int evaluation = 0;
    int perspective = 2 * (toPlay == white) - 1; // am I WTP (1) or BTP (-1)?

    // immobilized material
    U64 myImmMaterial = position[toPlay] & (enemyInfl | enemyChamInfl);
    while (myImmMaterial)
    {
        evaluation -= immBonus[pieceList[pop_lsb(myImmMaterial)]];
        myImmMaterial &= myImmMaterial - 1;
    }

    U64 enemyImmMaterial = position[notToPlay] & (myInfl | myChamInfl);
    while (enemyImmMaterial)
    {
        evaluation += immBonus[pieceList[pop_lsb(enemyImmMaterial)]];
        enemyImmMaterial &= enemyImmMaterial - 1;
    }


    // === IMMOBILIZER LINES OF SIGHT === //

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
    evaluation -= ((toPlay == white) * 140 + -perspective * imm_dist_penalty(myImmSq) + immLoSPen[myImmLoS]) * myImmImm * (myImmobilizer > 0);


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
    evaluation += ((notToPlay == white) * 140 + perspective * imm_dist_penalty(enemyImmSq) + immLoSPen[enemyImmLoS]) * enemyImmImm * (enemyImmobilizer > 0);


    // === KING COORDINATOR CAPTURE DISTANCE === //

    // if my king or coordinator end up in the corners, then there cannot be a penalty
    U64 enemyImmCorners = (enemyImmobilizer << 9 | enemyImmobilizer << 7 | enemyImmobilizer >> 7 | enemyImmobilizer >> 9) & enemyInfl;
    U64 myImmCorners = (myImmobilizer << 9 | myImmobilizer << 7 | myImmobilizer >> 7 | myImmobilizer >> 9) & myInfl;

    int myKCImm = (enemyImmCorners & (position[toPlay + coordinator] | position[toPlay + king])) > 0;
    int enemyKCImm = (myImmCorners & (position[notToPlay + coordinator] | position[notToPlay + king])) > 0;

    // an additional penalty for how long it takes for king-coordinator duo to capture at the given sq
    if (enemyImmobilizer && enemyImmImm && !myKCImm)
    {
        evaluation += kingCoordCaptPen(toPlay, enemyImmSq);
    }
    if (myImmobilizer && myImmImm && !enemyKCImm)
    {
        evaluation -= kingCoordCaptPen(notToPlay, myImmSq);
    }

    // evaluation += 200 * (enemyKCImm - myKCImm);


    // whoever has more material MUST be winning (not necessarily but y'know)
    return evaluation + perspective * materialScore + myMobilityScore - enemyMobilityScore;
}

const int kcPenalty[] =
{
    180, 140, 120, 100, 80, 60, 35, 20, 10, 5, 0
};

int kingCoordCaptPen(int stc, int sq)
{
    int notstc = !stc * 8;

    U64 enemyImmobilizer = position[notstc + immobilizer];
    int enemyImmSq = pop_lsb(enemyImmobilizer);
    U64 enemyInfl = (enemyImmobilizer > 0) * kingMoves[enemyImmSq];

    U64 totalBoard = position[white] | position[black];

    // coordinator mobility
    int kingSq = pop_lsb(position[stc + king]);
    U64 coordBoard = position[stc + coordinator];
    int coordSq = pop_lsb(coordBoard);

    U64 coordMob = get_rook_attacks(coordSq, totalBoard) | get_bishop_attacks(coordSq, totalBoard) | coordBoard & ~totalBoard;

    // king to file, coordinator to rank
    int kingFileDisp = (kingSq & 7) - (sq & 7);
    int kingFileDist = kingFileDisp * ((kingFileDisp > 0) - (kingFileDisp < 0));

    int validCoord = (coordBoard & ~enemyInfl) > 0;
    
    U64 sqRank = ranks[sq >> 3];
    int coordRankDist = 2 - ((coordMob & sqRank) > 0) - ((coordBoard & sqRank) > 0);
    int kfrr = (validCoord || coordBoard > 0 && coordRankDist == 0) * (kcPenalty[kingFileDist + coordRankDist]);

    // king to rank, coordinator to file
    int kingRankDisp = (kingSq >> 3) - (sq >> 3);
    int kingRankDist = kingRankDisp * ((kingRankDisp > 0) - (kingRankDisp < 0));

    U64 sqFile = files[sq & 7];
    int coordFileDist = 2 - ((coordMob & sqFile) > 0) - ((coordBoard & sqFile) > 0);
    int krrf = (validCoord || coordBoard > 0 && coordFileDist == 0) * (kcPenalty[kingRankDist + coordFileDist]);

    // return the largest penalty
    return kfrr < krrf ? krrf : kfrr;
}

int moveCaptureValue(Move m)
{
    switch (get_type(m))
    {
        case straddler:
            return
                pieceValues[get_c1(m)] +
                pieceValues[get_c2(m)] +
                pieceValues[get_c3(m)] +
                pieceValues[get_c4(m)];
        case retractor:
        case springer:
            return pieceValues[get_c1(m)];
        case coordinator:
            return
                pieceValues[get_c1(m)] +
                pieceValues[get_c2(m)];
        case immobilizer:
            return 0;
        case chameleon:
            return
                pieceValues[get_b_cu(m)] +
                pieceValues[get_b_cl(m) > 0] +
                pieceValues[get_b_cr(m) > 0] +
                pieceValues[get_b_cd(m) > 0] +

                (get_b_cd1(m) | get_b_cd2(m)) * pieceValues[coordinator] +
                get_b_cq(m) * pieceValues[retractor] +
                get_b_cn(m) * pieceValues[springer];
        case king:
            return
                pieceValues[get_c1(m)] +
                pieceValues[get_c2(m)] +
                pieceValues[get_c3(m)] +
                get_kb_c(m) * pieceValues[coordinator];
    }
    return 0;
}
