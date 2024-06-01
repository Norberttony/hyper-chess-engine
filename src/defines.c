
#include "defines.h"

U64 position[17];

const char pieceFEN[] = ".PQNRUBK.pqnrubk";
const char StartingFEN[] = "unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 1";

int toPlay = white;
int notToPlay = black;
int halfmove = 0;
int pieceListStore[80];
int* pieceList = pieceListStore + 8;

U64 zobristHashes[ZOBRIST_HASH_COUNT];
U64 zobristHash = 0;

U64 repeatTable[REPEAT_TABLE_ENTRIES];
int repeatTableIndex = 0;


void prettyPrintBoard()
{
    for (int r = 0; r < 8; r++)
    {
        printf(" %d  ", 8 - r);
        for (int f = 0; f < 8; f++)
        {
            int s = f + r * 8;

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
                if (position[p] & (1ULL << s))
                {
                    if (!isEmpty)
                    {
                        printf("\nCollision problem with %d\n", p);
                        return;
                    }
                    printf("%c ", pieceFEN[p]);
                    isEmpty = 0;
                }
            }

            // indicate that this is an empty square
            if (isEmpty)
            {
                printf("%c ", pieceFEN[0]);
            }
        }

        // next rank
        puts("");
    }

    // file names
    puts("    a b c d e f g h");

    printf("It is %s to play\n", toPlay == white ? "white": "black");

    printf("%llu\n", zobristHash);
}

int loadFEN(const char* fen)
{
    // clear all boards
    for (int i = 0; i < 17; i++)
    {
        position[i] = 0ULL;
    }

    // clear piece list
    for (int s = 0; s < 64; s++)
    {
        pieceList[s] = 0;
    }

    // clear zobrist hash
    zobristHash = 0ULL;

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
            // add the piece to the board
            int offset = isupper(fen[i]) ? white : black;
            position[val + offset] |= 1ULL << (f + r * 8);

            // do this on the occupancy board as well
            position[offset] |= 1ULL << (f + r * 8);

            // set piece list
            pieceList[f + r * 8] = val;

            // update zobrist hash
            zobristHash ^= get_zobrist_hash(f + r * 8, val, isupper(fen[i]));
            //printf("Added %llu\n", get_zobrist_hash(f + r * 8, val, isupper(fen[i])));

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
        toPlay = white;
        notToPlay = black;
        zobristHash ^= zobristHashes[ZOBRIST_HASH_COUNT - 1];
    }
    else if (fen[i] == 'b')
    {
        toPlay = black;
        notToPlay = white;
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
        halfmove = 0;
    }
    else if (isdigit(fen[i]))
    {
        halfmove = 2 * (int)(fen[i] - '0');
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
            if (pieceList[sq])
            {
                if (empty > 0)
                {
                    fen[index++] = '0' + empty;
                    empty = 0;
                }

                char pieceChar = pieceFEN[pieceList[sq]];
                if (position[black] & (1ULL << sq))
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

    if (toPlay == white)
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

            printf("%c ", pieceFEN[pieceList[s]]);
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
        repeats += repeatTable[j] == zobristHash;
    }

    return repeats >= 2;
}
