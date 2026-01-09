#include "position-defines.h"
#include <ctype.h> // for tolower and toupper
#include <stdlib.h>
#include <stdio.h>
#include "evaluate-defines.h"

PositionState g_states[MAX_GAME_LENGTH];

// 8 squares above and 8 squares below as extra padding
int pieceListStore[80];

Position g_pos =
{
    .toPlay = white,
    .notToPlay = black,
    .materialScore = { 0 },
    .fullmove = 0,
    .pieceList = pieceListStore + 8,
    .zobristHash = 0ULL,
    .state = g_states
};

const char pieceFEN[] = ".PQNRUBK.pqnrubk";
const char StartingFEN[] = "unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 0 1";

const char* squareNames[] =
{
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

U64 zobristHashes[ZOBRIST_HASH_COUNT];
U64 zobristHashes_halfmoves[ZOBRIST_HASH_COUNT_HALFMOVE + 1];

U64 repeatTable[REPEAT_TABLE_ENTRIES];
int repeatTableIndex = 0;
U64 repeatTableCopy[REPEAT_TABLE_ENTRIES];

void printBoardLine(void)
{
    puts("   +---+---+---+---+---+---+---+---+");
}

void prettyPrintBoard(void)
{
    printBoardLine();
    
    // first char is light square, second char is dark square
    const char* sqType = " :";

    for (int r = 0; r < 8; r++)
    {
        printf(" %d ", 8 - r);
        for (int f = 0; f < 8; f++)
        {
            int s = f + r * 8;

            char shade = sqType[(f + r) % 2];

            // search through all piece boards
            int isEmpty = 1;
            for (int p = 1; p <= 15; p++)
            {
                // skip, this board shows all white/black occupancy
                if (p == white || p == black)
                {
                    continue;
                }

                // bit test
                if (g_pos.boards[p] & (1ULL << s))
                {
                    if (!isEmpty)
                    {
                        printf("\nCollision problem with %d\n", p);
                        return;
                    }
                    printf("|%c%c%c", shade, pieceFEN[p], shade);
                    isEmpty = 0;
                }
            }

            // indicate that this is an empty square
            if (isEmpty)
            {
                printf("|%c%c%c", shade, shade, shade);
            }
        }

        // next rank
        puts("|");
        printBoardLine();
    }

    // file names
    puts("     a   b   c   d   e   f   g   h");

    printf("It is %s to play\n", g_pos.toPlay == white ? "white": "black");

    printf("%llu\n", g_pos.zobristHash);

    char fen[5000];
    int failure = getFEN(fen, 5000);
    if (failure)
    {
        puts("FEN: failed to get FEN");
    }
    else
    {
        printf("FEN: %s\n", fen);
    }
}

int loadFEN(const char* fen)
{
    // clear all boards
    for (int i = 0; i < 16; i++)
    {
        g_pos.boards[i] = 0ULL;
    }

    // clear piece list
    for (int s = 0; s < 64; s++)
    {
        g_pos.pieceList[s] = 0;
    }

    // clear repeat table
    for (int i = 0; i < REPEAT_TABLE_ENTRIES; i++)
    {
        repeatTable[i] = 0;
    }

    // clear scores
    g_pos.materialScore[0] = 0;
    g_pos.materialScore[1] = 0;

    // clear zobrist hash
    g_pos.zobristHash = 0ULL;

    // reset history
    g_pos.state = g_states;
    g_pos.state->halfmove = 0;
    g_pos.state->prevMove = 0;

    // interpret board string
    int i;
    int r = 0;
    int f = 0;
    for (i = 0; fen[i] != ' '; i++)
    {
        int val = convertFENToValue(fen[i]);
        if (fen[i] == '\0')
        {
            // woops! not enough spaces to delimit string! error!
            printf("Could not load invalid FEN string %s\n", fen);
            return 0;
        }
        else if (fen[i] == '/')
        {
            // onto the next rank!
            r++;
            f = 0;
        }
        else if (isdigit(fen[i]))
        {
            // skip this number of squares
            f += (int)(fen[i] - '0');
        }
        else if (val)
        {
            int sq = f + r * 8;

            // add the piece to the board
            int offset = isupper(fen[i]) ? white : black;
            g_pos.boards[val + offset] |= 1ULL << sq;

            // do this on the occupancy board as well
            g_pos.boards[offset] |= 1ULL << sq;

            // set piece list
            g_pos.pieceList[sq] = val;

            g_pos.materialScore[offset == black] += PSQT(val, offset, sq);

            // update zobrist hash
            g_pos.zobristHash ^= get_zobrist_hash(sq, val, isupper(fen[i]));

            // next square!
            f++;
        }
        else
        {
            // this must be an invalid character.
            printf("Invalid character in FEN string %s when parsing board; could not recognize '%c'\n", fen, fen[i]);
            return 0;
        }
    }

    // skip over space
    i++;

    // determine who it is to play
    if (fen[i] == 'w')
    {
        g_pos.toPlay = white;
        g_pos.notToPlay = black;
        g_pos.zobristHash ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];
    }
    else if (fen[i] == 'b')
    {
        g_pos.toPlay = black;
        g_pos.notToPlay = white;
    }
    else
    {
        // this must be an invalid character.
        printf("Invalid character in FEN string %s when parsing side-to-play; could not recognize '%c'\n", fen, fen[i]);
        return 0;
    }

    // skip over side-to-play and space
    i += 2;

    // determine halfmove counter
    if (fen[i] == '-')
    {
        g_pos.state->halfmove = 0;
    }
    else
    {
        g_pos.state->halfmove = atoi(fen + i);
    }

    // look for a space
    while (fen[i] != ' ')
    {
        if (fen[i] == '\0')
        {
            puts("FEN is missing a fullmove counter");
            return 0;
        }
        i++;
    }

    // skip over the space
    i++;

    // determine fullmove counter
    if (fen[i] == '-')
    {
        g_pos.fullmove = 0;
    }
    else
    {
        g_pos.fullmove = atoi(fen + i);
    }

    return 1;
}

int writeIntegerIntoString(int val, int bufferSize, char* str)
{
    int sign = val < 0;
    int digits = 0;
    int copy = val;
    do
    {
        digits++;
    }
    while (copy /= 10);

    if (digits + sign >= bufferSize)
    {
        puts("Could not write integer to string");
        return 0;
    }
    else
    {
        int idx = 0;
        if (sign)
        {
            str[idx++] = '-';
        }
        do
        {
            str[digits + sign - ++idx] = '0' + (val % 10);
        }
        while (val /= 10);
        return idx;
    }
}

int getFEN(char* fen, int bufsize)
{
    int index = 0;

    // set the piece configuration
    for (int r = 0; r < 8; r++)
    {
        int empty = 0;
        for (int f = 0; f < 8; f++)
        {
            int sq = f + r * 8;

            // add piece onto the board
            if (g_pos.pieceList[sq])
            {
                if (empty > 0)
                {
                    fen[index++] = '0' + empty;
                    if (index == bufsize)
                    {
                        return -1;
                    }
                    empty = 0;
                }

                char pieceChar = pieceFEN[g_pos.pieceList[sq]];
                if (g_pos.boards[black] & (1ULL << sq))
                {
                    pieceChar = tolower(pieceChar);
                }
                fen[index++] = pieceChar;
                if (index == bufsize)
                {
                    return -1;
                }
            }
            else
            {
                empty++;
            }
        }
        if (empty > 0)
        {
            fen[index++] = '0' + empty;
            if (index == bufsize)
            {
                return -1;
            }
        }
        if (r < 7)
        {
            fen[index++] = '/';
            if (index == bufsize)
            {
                return -1;
            }
        }
    }

    if (index + 3 >= bufsize)
    {
        return -1;
    }

    fen[index++] = ' ';

    // set side to play
    fen[index++] = g_pos.toPlay == white ? 'w' : 'b';

    fen[index++] = ' ';

    // set half move counter
    index += writeIntegerIntoString(g_pos.state->halfmove, bufsize - index, fen + index);

    if (index == bufsize)
    {
        return -1;
    }

    fen[index++] = ' ';

    if (index == bufsize)
    {
        return -1;
    }

    // set full move counter
    index += writeIntegerIntoString(g_pos.fullmove, bufsize - index, fen + index);

    if (index == bufsize)
    {
        return -1;
    }

    fen[index++] = '\0';
    
    if (index == bufsize)
    {
        return -1;
    }

    return 0;
}

int convertFENToValue(const char v)
{
    // get index of piece FEN
    for (int i = 0; pieceFEN[i] != '\0'; i++)
    {
        if (pieceFEN[i] == v)
        {
            // return correct value (wrapping)
            return i % 8;
            break;
        }
    }

    // no piece value
    return 0;
}

void printPieceList(void)
{
    puts("Piece list:");
    for (int r = 0; r < 8; r++)
    {
        printf(" %d  ", 8 - r);
        for (int f = 0; f < 8; f++)
        {
            int s = f + r * 8;

            printf("%c ", pieceFEN[g_pos.pieceList[s]]);
        }

        // next rank
        puts("");
    }

    // file names
    puts("    a b c d e f g h");
}

void generateZobristHashes(void)
{
    srand(5465875);
    for (int i = 0; i < ZOBRIST_HASH_COUNT; i++)
    {
        zobristHashes[i] = randomU64();
    }
    for (int i = 0; i < ZOBRIST_HASH_COUNT_HALFMOVE + 1; i++)
    {
        zobristHashes_halfmoves[i] = randomU64();
    }
}

int getNumberOfRepeats(void)
{
    int repeats = 0;
    for (int j = 0; j < REPEAT_TABLE_ENTRIES; j++)
    {
        repeats += repeatTable[j] == g_pos.zobristHash;
    }

    return repeats;
}

void saveRepeatTable(void)
{
    for (int i = 0; i < REPEAT_TABLE_ENTRIES; i++)
    {
        repeatTableCopy[i] = repeatTable[i];
    }
}

void restoreRepeatTable(void)
{
    for (int i = 0; i < REPEAT_TABLE_ENTRIES; i++)
    {
        repeatTable[i] = repeatTableCopy[i];
    }
}
