
#include "move-ordering.h"


Move orderFirst = 0;

const int orderFirstValue = 1000000000;
const int isCaptValue = 900000000;
const int killerValue = 800000000;

Move killerMoves[MAX_DEPTH][2] = { 0 };
int historyValues[2][64][64] = { 0 };


void orderMoves(Move* moves, int count, int depth)
{
    int scores[MAX_MOVES];

    Move* killers = &killer_move(depth, 0);

    // score the moves
    for (int i = 0; i < count; i++)
    {
        Move m = moves[i];
        int fromSq = get_from(m);
        int toSq = get_to(m);
        int isCapt = is_move_capt(m);
        int stmIdx = g_pos.toPlay == black;

        int orderFirstScore = orderFirstValue * (m == orderFirst);
        int captScore = isCapt * isCaptValue;

        int killScore = (m == killers[0] || m == killers[1]) * killerValue;
        int historyScore = historyValues[stmIdx][fromSq][toSq];

        int quietScore = !isCapt * (historyScore + killScore);

        scores[i] = orderFirstScore + captScore + quietScore;
        if (isCapt)
        {
            scores[i] += moveCaptureValue(m);
        }
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

void addKillerMove(Move m, int depth)
{
    int isStored = killer_move(depth, 0) == m;
    killer_move(depth, 1) = !isStored * killer_move(depth, 0) + isStored * killer_move(depth, 1);
    killer_move(depth, 0) = !isStored * m + isStored * killer_move(depth, 0);
}

void updateHistory(int from, int to, int bonus)
{
    // clamp bonus between -MAX_HISTORY and MAX_HISTORY to avoid oversaturated history values
    if (bonus > MAX_HISTORY)
    {
        bonus = MAX_HISTORY;
    }
    if (bonus < -MAX_HISTORY)
    {
        bonus = -MAX_HISTORY;
    }

    // apply the history gravity formula, which gives smaller bonuses if the history move was expected
    historyValues[g_pos.toPlay == black][from][to] += bonus - historyValues[g_pos.toPlay == black][from][to] * abs(bonus) / MAX_HISTORY;
}
