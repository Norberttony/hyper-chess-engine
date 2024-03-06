
#include "look-up-tables.h"

// [0] accesses 8th rank, [1] accesses 7th, etc.
U64 ranks[8];
U64 files[8];

// [0] accesses 8th rank, [1] accesses 7th, etc.
U64 sqRanks[64];
U64 sqFiles[64];

U64 kingMoves[64];
U64 deathSquares[64][64][2];

// where the springer captured given where it started and where it moved to
U64 springerCaptures[64][64];

// where the springer has to leap to to capture enemy
// springerLeaps[springerSq][enemySq]
U64 springerLeaps[64][64];

U64 retractorCaptures[64][64];


static const U64 not_a_file = 18374403900871474942ULL;
static const U64 not_h_file =  9187201950435737471ULL;


void populateRanksAndFiles()
{
    for (int i = 0; i < 8; i++)
    {
        // get ith rank
        U64 rank = 0ULL;
        for (int f = 0; f < 8; f++)
        {
            rank |= 1ULL << (i * 8 + f);
        }
        ranks[i] = rank;

        // populate sqRanks
        for (int f = 0; f < 8; f++)
        {
            sqRanks[i * 8 + f] = rank;
        }

        // get ith file
        U64 file = 0ULL;
        for (int r = 0; r < 8; r++)
        {
            file |= 1ULL << (r * 8 + i);
        }
        files[i] = file;

        // populate sqFiles
        for (int r = 0; r < 8; r++)
        {
            sqFiles[r * 8 + i] = file;
        }
    }
}

U64 genDeathSquares1(int sq1, int sq2)
{
    // get files and ranks of both squares
    int f1 = sq1 % 8;
    int r1 = sq1 / 8;

    int f2 = sq2 % 8;
    int r2 = sq2 / 8;
    
    // if on same file/rank, no death squares
    if (f1 == f2 || r1 == r2)
    {
        return 0ULL;
    }

    // use ranks and files bitboard to find intersections
    return ranks[r1] & files[f2];
}

U64 genDeathSquares2(int sq1, int sq2)
{
    // get files and ranks of both squares
    int f1 = sq1 % 8;
    int r1 = sq1 / 8;

    int f2 = sq2 % 8;
    int r2 = sq2 / 8;
    
    // if on same file/rank, no death squares
    if (f1 == f2 || r1 == r2)
    {
        return 0ULL;
    }

    // use ranks and files bitboard to find intersections
    return ranks[r2] & files[f1];
}

void populateDeathSquares()
{
    for (int s1 = 0; s1 < 64; s1++)
    {
        for (int s2 = 0; s2 < 64; s2++)
        {
            deathSquares[s1][s2][0] = genDeathSquares1(s1, s2);
            deathSquares[s1][s2][1] = genDeathSquares2(s1, s2);
        }
    }
}

U64 genKingMoves(int sqIndex)
{
    U64 king = 1ULL << sqIndex;
    
    U64 attacks = 0;

    attacks |= king << 1;
    attacks |= king << 7;
    attacks |= king << 8;
    attacks |= king << 9;

    attacks |= king >> 1;
    attacks |= king >> 7;
    attacks |= king >> 8;
    attacks |= king >> 9;

    // check file to see if we have to factor in out of bounds
    if (sqIndex % 8 == 7)
    {
        attacks &= not_a_file;
    }
    else if (sqIndex % 8 == 0)
    {
        attacks &= not_h_file;
    }

    return attacks;
}

void populateKingMoves()
{
    for (int s = 0; s < 64; s++)
    {
        kingMoves[s] = genKingMoves(s);
    }
}

U64 genSpringerLeap(int springerSq, int enemySq)
{
    // get file and rank of both squares
    int fs = springerSq % 8;
    int rs = springerSq / 8;

    int fe = enemySq % 8;
    int re = enemySq / 8;

    // difference in rank and file
    int fd = fs - fe;
    int rd = rs - re;

    // enemy cannot occupy same square as springer
    if (fs == fe && rs == re)
    {
        return 0ULL;
    }

    // make sure enemy is aligned in some way...
    if (fs == fe)
    {
        // same file. different ranks.
        // if on the edge of the board, no capture
        if (re == 0 || re == 7)
        {
            return 0ULL;
        }
        else if (enemySq < springerSq)
        {
            return 1ULL << (enemySq - 8);
        }
        else
        {
            return 1ULL << (enemySq + 8);
        }
    }
    else if (rs == re)
    {
        // same rank. different files.
        // if on the edge of the board, no capture
        if (fe == 0 || fe == 7)
        {
            return 0ULL;
        }
        else if (enemySq < springerSq)
        {
            return 1ULL << (enemySq - 1);
        }
        else
        {
            return 1ULL << (enemySq + 1);
        }
    }
    // diagonal alignment
    else if (abs(rd) == abs(fd))
    {
        // no capture if on the edge of the board
        if (re == 0 || re == 7 || fe == 0 || fe == 7)
        {
            return 0ULL;
        }

        // determine which diagonal this is
        // rank values are reversed from U64's interpretation
        // up left
        if (rd > 0 && fd > 0)
        {
            return 1ULL << (enemySq - 9);
        }
        // up right
        else if (rd > 0 && fd < 0)
        {
            return 1ULL << (enemySq - 7);
        }
        // down left
        else if (rd < 0 && fd > 0)
        {
            return 1ULL << (enemySq + 7);
        }
        // down right
        else if (rd < 0 && fd < 0)
        {
            return 1ULL << (enemySq + 9);
        }
    }

    // no alignment, no attack
    return 0ULL;
}

void populateSpringerLeaps()
{
    for (int s = 0; s < 64; s++)
    {
        for (int e = 0; e < 64; e++)
        {
            springerLeaps[s][e] = genSpringerLeap(s, e);
        }
    }
}

U64 genSpringerCapture(int start, int end)
{
    // get file and rank of both squares
    int fs = start % 8;
    int rs = start / 8;

    int fe = end % 8;
    int re = end / 8;

    // difference in rank and file
    int fd = fs - fe;
    int rd = rs - re;

    // springer cannot start and end at the same square
    if (fs == fe && rs == re)
    {
        return 0ULL;
    }

    // make sure end is aligned in some way...
    if (fs == fe)
    {
        // same file. different ranks.
        if (end < start)
        {
            return 1ULL << (end + 8);
        }
        else
        {
            return 1ULL << (end - 8);
        }
    }
    else if (rs == re)
    {
        // same rank. different files.
        if (end < start)
        {
            return 1ULL << (end + 1);
        }
        else
        {
            return 1ULL << (end - 1);
        }
    }
    // diagonal alignment
    else if (abs(rd) == abs(fd))
    {
        // determine which diagonal this is
        // rank values are reversed from U64's interpretation
        // up left
        if (rd > 0 && fd > 0)
        {
            return 1ULL << (end + 9);
        }
        // up right
        else if (rd > 0 && fd < 0)
        {
            return 1ULL << (end + 7);
        }
        // down left
        else if (rd < 0 && fd > 0)
        {
            return 1ULL << (end - 7);
        }
        // down right
        else if (rd < 0 && fd < 0)
        {
            return 1ULL << (end - 9);
        }
    }

    // no alignment, no attack
    return 0ULL;
}

void populateSpringerCaptures()
{
    for (int s = 0; s < 64; s++)
    {
        for (int e = 0; e < 64; e++)
        {
            springerCaptures[s][e] = genSpringerCapture(s, e);
        }
    }
}

U64 genRetractorCapture(int startSq, int enemySq)
{
    U64 captures = 0ULL;

    int file = startSq % 8;
    int rank = startSq / 8;
    int diff = enemySq - startSq;

    // retractor makes moves away from the enemy
    captures |= 1ULL << (startSq - diff);

    // no wrapping around the board by moving
    if (file == 0)
    {
        captures &= not_h_file;
    }
    else if (file == 7)
    {
        captures &= not_a_file;
    }
    if (rank == 0)
    {
        captures &= ~ranks[7];
    }
    else if (rank == 7)
    {
        captures &= ~ranks[0];
    }

    // no wrapping around the board by capturing
    // aka retractor must make a 1 square move away from enemy
    int enemyFile = enemySq % 8;
    int enemyRank = enemySq / 8;
    if (abs(enemyFile - file) > 1 || abs(enemyRank - rank) > 1 || enemyFile == file && enemyRank == rank)
    {
        return 0ULL;
    }

    return captures;
}

void populateRetractorCaptures()
{
    for (int s = 0; s < 64; s++)
    {
        for (int e = 0; e < 64; e++)
        {
            retractorCaptures[s][e] = genRetractorCapture(s, e);
        }
    }
}
