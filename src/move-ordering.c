
#include "move-ordering.h"


Move orderFirst = 0;

const int orderFirstValue = 1000000000;
const int isCaptValue = 900000000;
const int killerValue = 800000000;

Move killerMoves[MAX_DEPTH][2] = { 0 };
int historyValues[2][8][64] = { 0 };


void orderMoves(Move* moves, int count, int depth)
{
    int scores[MAX_MOVES];

    // score the moves
    for (int i = 0; i < count; i++)
    {
        Move m = moves[i];
        int type = m & move_typeMask;
        int fromSq = (m & move_fromMask) >> 3;
        int toSq = (m & move_toMask) >> 9;
        int isCapt = (m & move_captMask) > 0;

        int killScore = killerValue * (killerMoves[depth][0] == m || killerMoves[depth][1] == m);

        scores[i] = orderFirstValue * (m == orderFirst) + moveCaptureValue(m) + isCapt * isCaptValue + !isCapt * (historyValues[toPlay == black][type][toSq] + killScore);
    }

    // sort the moves using insertion sort
    for (int i = 1; i < count; i++)
    {
        Move m = moves[i];
        int s = scores[i];

        int j;
        for (j = i - 1; j >= 0 && scores[j] < s; j--)
        {
            moves[j + 1] = moves[j];
            scores[j + 1] = scores[j];
        }

        moves[j + 1] = m;
        scores[j + 1] = s;
    }

    /*
    for (int i = 0; i < count; i++)
    {
        prettyPrintMove(moves[i]);
        printf("is valued at %d\n", scores[i]);
    }
    */
}
