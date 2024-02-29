
#include "defines.h"

U64 position[17];

const char pieceFEN[] = ".PQNRUBK.pqnrubk";
const char StartingFEN[] = "unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 1";

int toPlay = white;
int fullmove = 0;


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
}

void loadFEN(const char* fen)
{
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
            return;
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

            // next square!
            f++;
        }
        else
        {
            // this must be an invalid character.
            printf("Invalid character in FEN string %s when parsing board; could not recognize '%c'\n", fen, fen[i]);
            return;
        }
    }

    // skip over space
    i++;

    // determine who it is to play
    if (fen[i] == 'w')
    {
        toPlay = white;
    }
    else if (fen[i] == 'b')
    {
        toPlay = black;
    }
    else
    {
        // this must be an invalid character.
        printf("Invalid character in FEN string %s when parsing side-to-play; could not recognize '%c'\n", fen, fen[i]);
        return;
    }

    // skip over side-to-play and space
    i += 2;

    // determine fullmove counter
    if (fen[i] == '-')
    {
        fullmove = -1;
    }
    else if (isdigit(fen[i]))
    {
        fullmove = (int)(fen[i] - '0');
    }
    else
    {
        // this must be an invalid character.
        printf("Invalid character in FEN string %s when parsing full move counter; could not recognize '%c'\n", fen, fen[i]);
        return;
    }
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
