
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

    // immobilized material and penalties for badly-positioned immobilized pieces
    U64 myImmMaterial = position[toPlay] & (enemyInfl | enemyChamInfl);
    int enemQ = (position[notToPlay + retractor] & ~myInfl) > 0;
    int enemR = (position[notToPlay + coordinator] & ~myInfl) > 0;
    int myHalfRank = perspective * 3 + ((perspective + 1) >> 1);

    int enemCoordSq = pop_lsb(position[notToPlay + coordinator]);
    U64 enemCoordinatorMoves = (enemR * get_queen_attacks(enemCoordSq, totalBoard)) & ~totalBoard;
    U64 enemCoordinatorXray = enemR * get_queen_attacks(enemCoordSq, 0ULL);

    int enemRetrSq = pop_lsb(position[notToPlay + retractor]);
    U64 enemRetractorMoves = (enemQ * get_queen_attacks(enemRetrSq, totalBoard)) & ~totalBoard;
    U64 enemRetractorXray = enemQ * get_queen_attacks(enemRetrSq, 0ULL);

    int enemyKingSq = pop_lsb(position[notToPlay + king]);

    while (myImmMaterial)
    {
        int sq = pop_lsb(myImmMaterial);
        int piece = pieceList[sq];

        int qRank = get_rank(sq) - perspective;
        U64 qDanger = (kingMoves[sq] & ~totalBoard & ranks[qRank]) * (perspective * qRank < myHalfRank);
        int qCount = ((enemRetractorMoves & qDanger) > 0) +
                    ((enemRetractorXray & qDanger) > 0) +
                    3 * (qDanger > 0);

        U64 rankBoard = ranks[get_rank(sq)];
        int rCount = ((enemCoordinatorMoves & rankBoard) > 0) +
                    ((enemCoordinatorXray & rankBoard) > 0) +
                    2 * (get_file(enemyKingSq) == get_file(sq));

        evaluation -= (PSQT(piece, toPlay, sq) * (8 * qCount * (piece != immobilizer) + 5 * rCount * enemR)) / 200;
        evaluation -= immBonus[piece];

        myImmMaterial &= myImmMaterial - 1;
    }

    U64 enemyImmMaterial = position[notToPlay] & (myInfl | myChamInfl);
    int myQ = (position[toPlay + retractor] & ~enemyInfl) > 0;
    int myR = (position[toPlay + coordinator] & ~enemyInfl) > 0;
    int enemHalfRank = -perspective * 3 + ((-perspective + 1) >> 1);

    int myCoordSq = pop_lsb(position[toPlay + coordinator]);
    U64 myCoordinatorMoves = ((myR * get_queen_attacks(myCoordSq, totalBoard)) & ~totalBoard);
    U64 myCoordinatorXray = (myR * get_queen_attacks(myCoordSq, 0ULL));

    int myRetrSq = pop_lsb(position[toPlay + retractor]);
    U64 myRetractorMoves = ((myQ * get_queen_attacks(myRetrSq, totalBoard)) & ~totalBoard);
    U64 myRetractorXray = (myQ * get_queen_attacks(myRetrSq, 0ULL));

    int myKingSq = pop_lsb(position[toPlay + king]);

    while (enemyImmMaterial)
    {
        int sq = pop_lsb(enemyImmMaterial);
        int piece = pieceList[sq];
        
        int qRank = get_rank(sq) - perspective;
        U64 qDanger = (kingMoves[sq] & ~totalBoard & ranks[qRank]) * (perspective * qRank < enemHalfRank);
        int qCount = ((myRetractorMoves & qDanger) > 0) +
                    ((myRetractorXray & qDanger) > 0) +
                    3 * (qDanger > 0);

        U64 rankBoard = ranks[get_rank(sq)];
        int rCount = ((myCoordinatorMoves & rankBoard) > 0) +
                    ((myCoordinatorXray & rankBoard) > 0) +
                    2 * (get_file(myKingSq) == get_file(sq));

        evaluation += (PSQT(piece, notToPlay, sq) * (8 * qCount * (piece != immobilizer) + 5 * rCount * myR)) / 200;
        evaluation += immBonus[piece];

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

    // evaluation += 200 * (enemyKCImm - myKCImm);


    // whoever has more material MUST be winning (not necessarily but y'know)
    return evaluation + perspective * (materialScore[0] - materialScore[1]) + myMobilityScore - enemyMobilityScore;
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
