
#include "move-ordering.h"


Move orderFirst = 0;


// orders captures first, and also orders the last best move first.
int compareMoves(const void *a, const void *b)
{
    Move m1 = *((Move*)a);
    Move m2 = *((Move*)b);

    // order the orderFirst move the highest
    int val1 = (m2 == orderFirst) * move_captMask + (int)(m2 & move_captMask);
    int val2 = (m1 == orderFirst) * move_captMask + (int)(m1 & move_captMask);

    return val1 - val2;
}
