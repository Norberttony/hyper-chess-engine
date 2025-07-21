
#include "pins.h"

// inBetween[sq1][sq2] returns a bitboard where 1's indicate all of the squares that are in
// between sq1 and sq2 exclusive.
U64 g_inBetween[64][64];

U64 g_pinMasks[64] = { 0ULL };
U64 g_pinned = 0ULL;
U64 g_checkMask = __UINT64_MAX__;


void initPins(void)
{
    // clear out all pins
    g_checkMask = __UINT64_MAX__;
    g_pinned = 0ULL;
    for (int s = 0; s < 64; s++)
    {
        g_pinMasks[s] = __UINT64_MAX__;
    }

    // set up inBetween
    for (int s1 = 0; s1 < 64; s1++)
    {
        for (int s2 = 0; s2 < 64; s2++)
        {
            if (s1 == s2)
            {
                continue;
            }
            // we want all of the squares between s1 and s2, excluding s1 and s2.
            U64 inb = 0ULL;
            int r1 = get_rank(s1);
            int f1 = get_file(s1);
            int r2 = get_rank(s2);
            int f2 = get_file(s2);

            // simple approach to handle each potential case separately. While it's possible to do
            // things like move "small", left bound, right bound, and shift amount outside of these
            // if statements and handle it more generally, this code doesn't have to be fast and
            // this feels more readable and maintainable.
            if (f1 == f2)
            {
                // aligned by file, so shift by rank
                U64 small = r1 < r2 ? 1ULL << s1 : 1ULL << s2;
                small <<= 8;
                r1++;
                while (r1 < r2)
                {
                    inb |= small;
                    small <<= 8;
                    r1++;
                }
            }
            else if (r1 == r2)
            {
                // aligned by rank, so shift by file
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                small <<= 1;
                f1++;
                while (f1 < f2)
                {
                    inb |= small;
                    small <<= 1;
                    f1++;
                }
            }
            else if (f2 - f1 == r2 - r1)
            {
                // up left down right diagonal
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                small <<= 9;
                f1++;
                while (f1 < f2)
                {
                    inb |= small;
                    small <<= 9;
                    f1++;
                }
            }
            else if (f1 - f2 == r2 - r1)
            {
                // up left down right diagonal
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                small >>= 7;
                f1++;
                while (f1 < f2)
                {
                    inb |= small;
                    small >>= 7;
                    f1++;
                }
            }

            g_inBetween[s1][s2] = inb;
        }
    }
}

void generatePins(void)
{
    // clear previous pins
    while (g_pinned)
    {
        g_pinMasks[pop_lsb(g_pinned)] = __UINT64_MAX__;
        g_pinned &= g_pinned - 1;
    }
    g_checkMask = __UINT64_MAX__;

}
