
#include "move-ordering.h"


Move orderFirst = 0;

const int orderFirstValue = 1000000000;
const int isCaptValue = 900000000;
const int killerValue = 800000000;
// place losing captures after killer moves
const int losingCaptValue = killerValue;

Move killerMoves[MAX_DEPTH][2] = { 0 };
int historyValues[2][64][64] = { 0 };

int continuationHistory[CONT_HISTORY_PLY][7][64][7][64] = { 0 };


void orderMoves(Move* moves, int count, int height, int depth)
{
    int scores[MAX_MOVES];

    Move* killers = &killer_move(height, 0);

    Move prevs[CONT_HISTORY_PLY] = { 0 };
    for (int i = 0; i < CONT_HISTORY_PLY; i++)
    {
        Move m = get_move_n_ply_ago(i + 1);
        if (m)
        {
            prevs[i] = m;
        }
    }


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

        if (!isCapt)
        {
            // add continuation history value
            for (int i = 0; i < CONT_HISTORY_PLY; i++)
            {
                Move pm = prevs[i];
                if (pm)
                {
                    historyScore += continuationHistory[i][get_type(pm) - 1][get_to(pm)][get_type(m) - 1][get_to(m)];
                }
            }
        }

        int quietScore = !isCapt * (historyScore + killScore);

        scores[i] = orderFirstScore + captScore + quietScore;
        if (isCapt)
        {
            int captVal = moveCaptureValue(m);
            scores[i] += captVal;
            // sort losing captures
            if (depth > 3 && captVal < pieceValues[get_type(m)] && isSquareControlled(g_pos.notToPlay, toSq, get_type(m)))
            {
                scores[i] += losingCaptValue - isCaptValue - pieceValues[get_type(m)];
            }
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

void addKillerMove(Move m, int height)
{
    int isStored = killer_move(height, 0) == m;
    killer_move(height, 1) = !isStored * killer_move(height, 0) + isStored * killer_move(height, 1);
    killer_move(height, 0) = !isStored * m + isStored * killer_move(height, 0);
}

void updateHistory(Move m, int bonus)
{
    int from = get_from(m);
    int to = get_to(m);

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
    int s = g_pos.toPlay == black;
    historyValues[s][from][to] += bonus - historyValues[s][from][to] * abs(bonus) / MAX_HISTORY;

    // continuation history
    for (int i = 0; i < CONT_HISTORY_PLY; i++)
    {
        Move pm = get_move_n_ply_ago(i + 1);
        if (pm)
        {
            int* v = &continuationHistory[i][get_type(pm) - 1][get_to(pm)][get_type(m) - 1][to];
            *v += bonus - *v * abs(bonus) / MAX_HISTORY;
        }
    }
}
