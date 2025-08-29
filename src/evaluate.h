#ifndef EVALUATE_HEADER
#define EVALUATE_HEADER

#include "evaluate-defines.h"
#include "defines.h"
#include "look-up-tables.h"
#include "magic-bitboards.h"
#include "move.h"


#define imm_dist_penalty(sq) (20 * (sq >> 3))


extern int EVAL_DBG_PRINT;


// greedy evaluation that counts material based on piece values
int evaluate(void);

// determines the material value of all the pieces captured.
int moveCaptureValue(Move m);


// returns the value of an immobilizer being on the given square based on immBonus
static inline int getImmobilizedValue(int sq, int stm)
{
    // immobilizd enemy material
    U64 imm = kingMoves[sq] & g_pos.boards[!stm * 8];

    int score = 0;
    while (imm)
    {
        score += immBonus[g_pos.pieceList[pop_lsb(imm)]];
        imm &= imm - 1;
    }

    // apply penalty if the imagined immobilizer would be immobilized by enemy pieces.
    U64 enemImmBoard = g_pos.boards[g_pos.notToPlay + immobilizer];
    U64 enemChamBoard = g_pos.boards[g_pos.notToPlay + chameleon];
    int willBeImm = (kingMoves[sq] & (enemImmBoard | enemChamBoard)) > 0;
    score -= willBeImm * immBonus[immobilizer];

    return score;
}

#endif
