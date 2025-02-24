
#include "evaluate.h"

struct EvalContext {
    U64 totalBoard;
    U64 immobilizers[2];
    U64 infl[2];
    int immSq[2];
    int immImm[2];
};

enum
{
    mine, enemy
};


// counts number of pseudo-legal moves that the side can perform given the pieces.
// does not actually work for king or straddler.
static inline __attribute__((always_inline)) int evalMobility(struct EvalContext *ctx, int reverseSide, int pieceType)
{
    int side = !reverseSide * toPlay + reverseSide * notToPlay;

    int mobility = 0;
    U64 totalBoard = ctx->totalBoard;

    U64 myBoard = position[side + pieceType] & ~ctx->infl[(enemy + reverseSide) & 1];
    while (myBoard)
    {
        int sq = pop_lsb(myBoard);
        U64 moveBoard = (get_rook_attacks(sq, totalBoard) | get_bishop_attacks(sq, totalBoard)) & ~totalBoard;

        while (moveBoard)
        {
            mobility++;
            moveBoard &= moveBoard - 1;
        }

        myBoard &= myBoard - 1;
    }

    return mobility;
}

// immobilized material and penalties for badly-positioned immobilized pieces
static inline __attribute__((always_inline)) int evalImmPieces(struct EvalContext *ctx, int reverseSide, int perspective)
{
    int side = !reverseSide * toPlay + reverseSide * notToPlay;
    int notSide = !side * 8;

    int mineValue = (mine + reverseSide) & 0x1;
    int enemyValue = (enemy + reverseSide) & 0x1;

    U64 totalBoard = ctx->totalBoard;
    U64 myInfl = ctx->infl[mineValue];
    U64 enemyInfl = ctx->infl[enemyValue];

    int evaluation = 0;

    U64 enemyChamInfl = ctx->immobilizers[mineValue] * ((myInfl & position[notSide + chameleon]) > 0);

    U64 myImmMaterial = position[side] & (enemyInfl | enemyChamInfl);
    int enemQ = (position[notSide + retractor] & ~myInfl) > 0;
    int enemR = (position[notSide + coordinator] & ~myInfl) > 0;
    int myHalfRank = perspective * 3 + ((perspective + 1) >> 1);

    int enemCoordSq = pop_lsb(position[notSide + coordinator]);
    U64 enemCoordinatorMoves = (enemR * get_queen_attacks(enemCoordSq, totalBoard)) & ~totalBoard;
    U64 enemCoordinatorXray = enemR * get_queen_attacks(enemCoordSq, 0ULL);

    int enemRetrSq = pop_lsb(position[notSide + retractor]);
    U64 enemRetractorMoves = (enemQ * get_queen_attacks(enemRetrSq, totalBoard)) & ~totalBoard;
    U64 enemRetractorXray = enemQ * get_queen_attacks(enemRetrSq, 0ULL);

    int enemyKingSq = pop_lsb(position[notSide + king]);

    while (myImmMaterial)
    {
        int sq = pop_lsb(myImmMaterial);
        int piece = pieceList[sq];

        // finds three forward squares of piece, and determines if they are on the opponent's side of the territory.
        // The first three forward squares are most likely to be in the opponent's territory, AND are most vulnerable to a retractor.
        int qRank = get_rank(sq) - perspective;
        U64 qDanger = (kingMoves[sq] & ~totalBoard & ranks[qRank]) * (perspective * qRank < myHalfRank);

        // the vulnerability existing is by itself dangerous, but for a more continuous eval and to
        // encourage the engine to make progress in capturing the immobilized pieces, 
        int qCount = ((enemRetractorMoves & qDanger) > 0) +     // retractor can move to vulnerable squares
                    ((enemRetractorXray & qDanger) > 0) +       // retractor x-rays vulnerable squares
                    3 * (qDanger > 0);                          // vulnerability exists

        // similarly here, punish vulnerable rank squares that the opponent's coordinator can easily access.
        U64 rankBoard = ranks[get_rank(sq)];
        int rCount = ((enemCoordinatorMoves & rankBoard) > 0) +
                    ((enemCoordinatorXray & rankBoard) > 0) +
                    2 * (get_file(enemyKingSq) == get_file(sq));

        // bonus based on how far away retractor or coordinator are to capture this piece.
        evaluation -= (PSQT(piece, side, sq) * (8 * qCount * (piece != immobilizer) + 5 * rCount * enemR)) / 200;

        // flat bonus for piece being unable to move.
        evaluation -= immBonus[piece];

        myImmMaterial &= myImmMaterial - 1;
    }

    return evaluation;
}

static inline __attribute__((always_inline)) int evalImmLoS(struct EvalContext *ctx, U64 diags, U64 lines, int reverseSide, int perspective)
{
    int mineValue = (mine + reverseSide) & 0x1;

    U64 myImmobilizer = ctx->immobilizers[mineValue];
    int myImmImm = ctx->immImm[mineValue];

    // determines whether to test left to right (-1) or up to down (1)
    int testUpDn = myImmobilizer & ranks[0] | myImmobilizer & ranks[7];
    int testLtRt = myImmobilizer & files[7] | myImmobilizer & files[0];
    int onlyTest = testUpDn - testLtRt;

    // count lines of sight for the immobilizer
    int myImmLoS = 
        // top left to bottom right diagonal
        (((myImmobilizer << 9 | myImmobilizer >> 9) & diags) > 0) * (onlyTest == 0) +
        // up to down
        (((myImmobilizer << 8 | myImmobilizer >> 8) & lines) > 0) * (onlyTest >= 0) +
        // top right to bottom left
        (((myImmobilizer << 7 | myImmobilizer >> 7) & diags) > 0) * (onlyTest == 0) +
        // left to right
        (((myImmobilizer << 1 | myImmobilizer >> 1) & lines) > 0) * (onlyTest <= 0);

    // handle a corner case (well, literally, the immobilizer being in the corner)
    myImmLoS *= !(testUpDn && testLtRt);

    // apply penalty based on the number of available lines of attack
    return -immLoSPen[myImmLoS] * myImmImm * (myImmobilizer > 0);
}

const int immDistPenalties[8] =
{
    0, 5, 10, 30, 50, 70, 70, 70
};
static inline __attribute__((always_inline)) int evalImmDist(struct EvalContext* ctx, int reverseSide, int perspective)
{
    int mineValue = (mine + reverseSide) & 0x1;

    U64 myImmobilizer = ctx->immobilizers[mineValue];
    int myImmSq = ctx->immSq[mineValue];
    int myImmImm = ctx->immImm[mineValue];

    return (myImmobilizer > 0) * myImmImm * -immDistPenalties[(perspective == 1) * 7 + -perspective * get_rank(myImmSq)];
}

int evaluate()
{
    U64 enemyImmobilizer = position[notToPlay + immobilizer];
    int enemyImmSq = pop_lsb(enemyImmobilizer);
    U64 enemyInfl = (enemyImmobilizer > 0) * kingMoves[enemyImmSq];

    U64 myImmobilizer = position[toPlay + immobilizer];
    int myImmSq = pop_lsb(myImmobilizer);
    U64 myInfl = (myImmobilizer > 0) * kingMoves[myImmSq];

    // if immobilizer is immobilized, then it shouldn't be evaluated highly
    int myImmImm = (myInfl & (position[notToPlay + chameleon] | position[notToPlay + immobilizer])) > 0;
    int enemyImmImm = (enemyInfl & (position[toPlay + chameleon] | position[toPlay + immobilizer])) > 0;


    struct EvalContext ctx =
    {
        .totalBoard = position[white] | position[black],
        .immobilizers =
        {
            myImmobilizer, enemyImmobilizer
        },
        .infl =
        {
            myInfl, enemyInfl
        },
        .immSq =
        {
            myImmSq, enemyImmSq
        },
        .immImm =
        {
            myImmImm, enemyImmImm
        }
    };

    int evaluation = 0;

    // count mobility
    U64 totalBoard = position[toPlay] | position[notToPlay];
    for (int i = 2; i <= 6; i++)
    {
        evaluation += evalMobility(&ctx, 0, i);
        evaluation -= evalMobility(&ctx, 1, i);
    }
    
    int perspective = 2 * (toPlay == white) - 1; // am I WTP (1) or BTP (-1)?

    evaluation += evalImmPieces(&ctx, 0, perspective);
    evaluation -= evalImmPieces(&ctx, 1, -perspective);

    evaluation += evalImmLoS(&ctx, totalBoard, totalBoard, 0, perspective);
    evaluation -= evalImmLoS(&ctx, totalBoard, totalBoard, 1, -perspective);

    evaluation += evalImmDist(&ctx, 0, perspective);
    evaluation -= evalImmDist(&ctx, 1, -perspective);

    // whoever has more material MUST be winning (not necessarily but y'know)
    return evaluation + perspective * (materialScore[0] - materialScore[1]);
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
