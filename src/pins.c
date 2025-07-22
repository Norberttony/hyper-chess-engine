
#include "pins.h"

// inBetween[sq1][sq2] returns a bitboard where 1's indicate all of the squares that are in
// between sq1 and sq2 exclusive.
U64 g_inBetween[64][64];

U64 g_pinMasks[64] = { 0ULL };
U64 g_pinned = 0ULL;
U64 g_checkMask = __UINT64_MAX__;
U64 g_immCheckMask = __UINT64_MAX__;


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
                int l = r1 < r2 ? r1 : r2;
                int r = r1 < r2 ? r2 : r1;
                small <<= 8;
                l++;
                while (l < r)
                {
                    inb |= small;
                    small <<= 8;
                    l++;
                }
            }
            else if (r1 == r2)
            {
                // aligned by rank, so shift by file
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                int l = f1 < f2 ? f1 : f2;
                int r = f1 < f2 ? f2 : f1;
                small <<= 1;
                l++;
                while (l < r)
                {
                    inb |= small;
                    small <<= 1;
                    l++;
                }
            }
            else if (f2 - f1 == r2 - r1)
            {
                // up left down right diagonal
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                int l = f1 < f2 ? f1 : f2;
                int r = f1 < f2 ? f2 : f1;
                small <<= 9;
                l++;
                while (l < r)
                {
                    inb |= small;
                    small <<= 9;
                    l++;
                }
            }
            else if (f1 - f2 == r2 - r1)
            {
                // up left down right diagonal
                U64 small = f1 < f2 ? 1ULL << s1 : 1ULL << s2;
                int l = f1 < f2 ? f1 : f2;
                int r = f1 < f2 ? f2 : f1;
                small >>= 7;
                l++;
                while (l < r)
                {
                    inb |= small;
                    small >>= 7;
                    l++;
                }
            }

            g_inBetween[s1][s2] = inb;
        }
    }
}

void applyPin(U64 ib, U64 totalBoard, int pinnerSq, int kingSq, int immSq)
{
    U64 immInfl = immSq == -1 ? 0ULL : kingMoves[immSq];
    U64 pinned = ib & totalBoard;

    int singlePinner = (pinned & (pinned - 1)) == 0ULL;
    int isImmPinned = pinned == (1ULL << immSq);

    if (!pinned)
    {
        // even without any pieces to block a springer check, this might not be check,
        // maybe the attacking piece is immobilized.
        if ((1ULL << pinnerSq) & immInfl)
        {
            g_pinned |= 1ULL << immSq;
            g_pinMasks[immSq] &= ib | kingMoves[pinnerSq];
        }
        else
        {
            // ok, this must be check then.
            g_checkMask &= ib;
            g_immCheckMask &= ib | kingMoves[pinnerSq];
        }
    }
    // ensure there is only one pinned piece, and that the pinner is not immobilized
    else if (singlePinner && (!(immInfl & (1ULL << pinnerSq)) || isImmPinned))
    {
        // if we are pinning the immobilizer, it can still move around the pinner because
        // then it's just immobilizing it.
        if (isImmPinned)
        {
            ib |= kingMoves[pinnerSq];
        }

        g_pinned |= pinned;
        g_pinMasks[pop_lsb(pinned)] &= ib;
    }
}

// handles a square (pinSq) being pinned by a straddler.
void handleStraddlerPins(int pinSq, U64 totalBoard, int kingSq, int immSq)
{
    U64 pinBoard = 1ULL << pinSq;
    U64 enem = g_pos.boards[g_pos.notToPlay];
    U64 enemStra = g_pos.boards[g_pos.notToPlay | straddler];
    U64 pinners = get_rook_attacks(pinSq, enem | (1ULL << kingSq)) & enemStra;
    while (pinners)
    {
        int pSq = pop_lsb(pinners);
        applyPin(pinBoard | g_inBetween[pinSq][pSq], totalBoard, pSq, kingSq, immSq);
        pinners &= pinners - 1;
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

    U64 kingBoard = g_pos.boards[g_pos.toPlay | king];
    int kingSq = pop_lsb(kingBoard);

    U64 totalBoard = g_pos.boards[white] | g_pos.boards[black];
    U64 immBoard = g_pos.boards[g_pos.toPlay | immobilizer];
    int immSq = immBoard == 0ULL ? -1 : pop_lsb(immBoard);

    // springer pins
    U64 sprBoard = g_pos.boards[g_pos.notToPlay | springer];
    while (sprBoard)
    {
        int sprSq = pop_lsb(sprBoard);
        U64 land = springerLeaps[sprSq][kingSq];

        // if the springer can land, it means that it has the king in its line of sight.
        if (land)
        {
            applyPin(g_inBetween[sprSq][kingSq] | land, totalBoard, sprSq, kingSq, immSq);
        }
        sprBoard &= sprBoard - 1;
    }

    // retractor pins
    U64 retrBoard = g_pos.boards[g_pos.notToPlay | retractor];
    if (kingMoves[kingSq] & retrBoard){
        int retrSq = pop_lsb(retrBoard);
    
        U64 land = retractorCaptures[retrSq][kingSq];
        if (land)
        {
            applyPin(land, totalBoard, retrSq, kingSq, immSq);
        }
    }

    // straddler pins
    U64 straBoard = g_pos.boards[g_pos.notToPlay | straddler];
    int kingFile = get_file(kingSq);
    if (kingFile > 0 && kingFile < 7)
    {
        if ((kingBoard >> 1) & straBoard)
        {
            // check for straddler pins on the right side
            handleStraddlerPins(kingSq + 1, totalBoard, kingSq, immSq);
        }
        else if ((kingBoard << 1) & straBoard)
        {
            // left side
            handleStraddlerPins(kingSq - 1, totalBoard, kingSq, immSq);
        }
    }
    int kingRank = get_rank(kingSq);
    if (kingRank > 0 && kingRank < 7)
    {
        if ((kingBoard >> 8) & straBoard)
        {
            // up
            handleStraddlerPins(kingSq + 8, totalBoard, kingSq, immSq);
        }
        else if ((kingBoard << 8) & straBoard)
        {
            // down
            handleStraddlerPins(kingSq - 8, totalBoard, kingSq, immSq);
        }
    }
}

void debugPrintPins(void)
{
    puts("Printing debug info for pins...");

    // print each of the pinned pieces
    U64 pinned = g_pinned;
    while (pinned)
    {
        int sq = pop_lsb(pinned);
        printf("Piece on square %s is pinned according to this mask:\n", squareNames[sq]);
        printBitboard(g_pinMasks[sq]);
        pinned &= pinned - 1;
    }

    // print the check mask
    puts("Because of checks (or lack thereof) each piece must abide to this check mask:");
    printBitboard(g_checkMask);

    // print the immobilizer's check mask
    puts("Of course, the immobilizer follows a different check mask:");
    printBitboard(g_immCheckMask);
}
