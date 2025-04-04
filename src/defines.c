
#include "defines.h"

// 8 squares above and 8 squares below as extra padding
int pieceListStore[80];

Position g_pos =
{
    .toPlay = white,
    .notToPlay = black,
    .materialScore = { 0 },
    .halfmove = 0,
    .pieceList = pieceListStore + 8,
    .zobristHash = 0ULL
};

const char pieceFEN[] = ".PQNRUBK.pqnrubk";
const char StartingFEN[] = "unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 1";

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

U64 repeatTable[REPEAT_TABLE_ENTRIES];
int repeatTableIndex = 0;


void printBoardLine()
{
    puts("   +---+---+---+---+---+---+---+---+");
}

void prettyPrintBoard()
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
        g_pos.halfmove = 0;
    }
    else if (isdigit(fen[i]))
    {
        g_pos.halfmove = 2 * (int)(fen[i] - '0');
    }
    else
    {
        // this must be an invalid character.
        printf("Invalid character in FEN string %s when parsing full move counter; could not recognize '%c'\n", fen, fen[i]);
        return 0;
    }

    return 1;
}

char* getFEN()
{
    char* fen = (char*)malloc(sizeof(char) * 2000);
    int index = 0;

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
                    empty = 0;
                }

                char pieceChar = pieceFEN[g_pos.pieceList[sq]];
                if (g_pos.boards[black] & (1ULL << sq))
                {
                    pieceChar = tolower(pieceChar);
                }
                fen[index++] = pieceChar;
            }
            else
            {
                empty++;
            }
        }
        if (empty > 0)
        {
            fen[index++] = '0' + empty;
        }
        if (r < 7)
            fen[index++] = '/';
    }

    fen[index++] = ' ';

    if (g_pos.toPlay == white)
    {
        fen[index++] = 'w';
    }
    else
    {
        fen[index++] = 'b';
    }

    fen[index++] = ' ';

    fen[index++] = '-';
    fen[index++] = '\0';

    return fen;
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

void printPieceList()
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

void generateZobristHashes()
{
    srand(5465875);
    for (int i = 0; i < ZOBRIST_HASH_COUNT; i++)
    {
        zobristHashes[i] = randomU64();
    }
}

int getThreefoldFlag()
{
    int repeats = 0;
    for (int j = 0; j < REPEAT_TABLE_ENTRIES; j++)
    {
        repeats += repeatTable[j] == g_pos.zobristHash;
    }

    return repeats >= 2;
}
