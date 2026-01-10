#include "uci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "../platform/platform.h"
#include "../search/move-ordering.h"
#include "../search/search-defines.h"
#include "../search/transposition-table.h"
#include "../search/think.h"
#include "../search/perft.h"
#include "../movegen/position-defines.h"
#include "../debug/test-suite.h"
#include "../debug/debug.h"

#define INPUT_BUFFER 5000

static SearchResults res;

void uciOk(void);
void parseGo(char* line);
void parsePos(char* line);

void uciLoop(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    FILE* inputFile = stdin;

    char line[INPUT_BUFFER];
    uciOk();

    while (1)
    {
        fflush(stdout);

        // according to UCI, every command must end with a line break.
        if (readLine(line, INPUT_BUFFER, inputFile) || line[0] == '\n')
        {
            if (inputFile != stdin)
            {
                fclose(inputFile);
                inputFile = stdin;
            }
            continue;
        }

        if (inputFile != stdin)
        {
            printf("%s", line);
        }

        if (!strncmp(line, "isready", 7))
        {
            puts("readyok");
        }
        else if (!strncmp(line, "position ", 9))
        {
            parsePos(line);
        }
        else if (!strncmp(line, "ucinewgame", 10))
        {
            parsePos("position startpos\n");
        }
        else if (!strncmp(line, "go", 2))
        {
            parseGo(line);
        }
        else if (!strncmp(line, "quit", 4))
        {
            res.stopThinking = -1;
            break;
        }
        else if (!strncmp(line, "uci", 3))
        {
            uciOk();
        }
        else if (!strncmp(line, "clear hash", 10))
        {
            memset(transpositionTable, 0, sizeof(transpositionTable));
            memset(historyValues, 0, sizeof(historyValues));
            memset(killerMoves, 0, sizeof(killerMoves));
            memset(continuationHistory, 0, sizeof(continuationHistory));
            count_TT_clear();
        }
        else if (!strncmp(line, "runtestsuite", 12))
        {
            runTestSuite();
        }
        else if (!strncmp(line, "readfile", 8))
        {
            if (inputFile != stdin)
            {
                fclose(inputFile);
            }

            // removes the line break character at the end of the line.
            for (int i = 9; i < INPUT_BUFFER; i++)
            {
                if (line[i] == '\0')
                {
                    line[i - 1] = '\0';
                    break;
                }
            }
            printf("Trying to open file %s...\n", &line[9]);

            inputFile = fopen(&line[9], "r");
            if (!inputFile)
            {
                inputFile = stdin;
                puts("File not found");
            }
        }
        else if (!strncmp(line, "d\n", 3))
        {
            prettyPrintBoard();
        }
        else if (!strncmp(line, "setoption ", 10))
        {
            // set line to all lowercase and remove any repeated spaces
            char cleaned[INPUT_BUFFER];
            int j = 0;
            char prev = 'a';
            for (int i = 0; ; i++)
            {
                if (line[i] == ' ' && prev == ' ')
                {
                    continue;
                }
                cleaned[j++] = tolower(line[i]);
                if (line[i] == '\0')
                {
                    break;
                }
            }

            // read the name, and based on that, the value.
            if (!strncmp(cleaned + 10, "name hash value ", 16))
            {
                int mb = atoi(cleaned + 26);
                setTranspositionTableSize(mb);
            }
        }

        if (inputFile == stdin)
        {
#ifdef DEBUG
            // if the debug part of the program is being tested OR if the file was just closed.
            count_print();
#endif
        }

        if (res.stopThinking == -1)
        {
            break;
        }
    }

    if (inputFile != stdin)
    {
        // if the "quit" command was found in a file, the debug results should be printed.
#ifdef DEBUG
        count_print();
#endif
        fclose(inputFile);
    }
}

void uciOk(void)
{
    puts("id name Hyper Active");
    puts("id author Norbert Krajewski");
    printf("option name Hash type spin min 1 max %d\n", MAX_TT_SIZE_MB);
    puts("uciok");
}

void parseGo(char* line)
{
    int toPlay = g_pos.toPlay;

    int depth = -1;
    int movesToGo = 20;
    int moveTime = -1;
    int time = -1;
    int inc = 0;

    // match various parameters given by the user
    char* matchAt = NULL;

    if ((matchAt = strstr(line, "perft")))
    {
        puts("Move | Node count | Capture nodes | Pieces captured | Checkmates");
        divide(atoi(matchAt + 6), 0);
        return;
    }

    if ((matchAt = strstr(line, "depth")))
    {
        depth = atoi(matchAt + 6);
    }

    if ((matchAt = strstr(line, "movestogo")))
    {
        movesToGo = atoi(matchAt + 10);
    }
    if ((matchAt = strstr(line, "movetime")))
    {
        moveTime = atoi(matchAt + 9);
    }

    if ((matchAt = strstr(line, "wtime")) && toPlay == white)
    {
        time = atoi(matchAt + 6);
    }
    if ((matchAt = strstr(line, "btime")) && toPlay == black)
    {
        time = atoi(matchAt + 6);
    }
    if ((matchAt = strstr(line, "winc")) && toPlay == white)
    {
        inc = atoi(matchAt + 5);
    }
    if ((matchAt = strstr(line, "binc")) && toPlay == black)
    {
        inc = atoi(matchAt + 5);
    }

    memset(&res, 0, sizeof(res));

    // interpret the commands

    // think infinitely by default
    SearchParams params =
    {
        .thinkStart = getCurrentTime(),
        .thinkingTime = -1
    };

    if (moveTime != -1)
    {
        time = moveTime;
        movesToGo = 1;
    }

    if (time != -1)
    {
        time /= movesToGo;
        time -= 15; // some buffer to prevent bot from timing out
        if (time < 0)
        {
            time = 0;
        }
        params.thinkingTime = time + inc / 2;
    }

    if (depth == -1)
    {
        params.maxDepth = MAX_DEPTH;
    }
    else
    {
        params.maxDepth = depth;
    }

    startThink(&params, &res);
}

void parsePos(char* line)
{
    // where the cursor currently is.
    int idx = 9;

    // determine what FEN to load
    if (!strncmp(&line[idx], "startpos", 8))
    {
        loadFEN(StartingFEN);
        idx += 9;
    }
    else if (!strncmp(&line[idx], "fen", 3))
    {
        idx += 4;

        // extract FEN string
        char fen[INPUT_BUFFER];
        int spaces = 0;
        int startIdx = idx;
        for (; line[idx] != '\n'; idx++)
        {
            fen[idx - startIdx] = line[idx];
            spaces += line[idx] == ' ';
            if (spaces == 4)
            {
                break;
            }
        }
        loadFEN(fen);
        idx++;
    }

    if (!strncmp(&line[idx], "moves", 5))
    {
        idx += 6;
        loadLAN(&line[idx]);
    }
}

void pollStop(void)
{
    if (isLineWaiting())
    {
        char input[256] = "";
        readLine(input, 256, stdin);

        if (!strncmp(input, "quit", 4) || !strncmp(input, "stop", 4))
        {
            res.stopThinking = 1;
        }
    }
}

void loadLAN(char* str)
{
    for (int i = 0; str[i] != '\0' && str[i + 1] != '\0'; i += 5)
    {
        // get coordinates of from square and to square
        int fromF = str[i] - 'a';
        int fromR = 7 - (str[i + 1] - '1');

        int toF = str[i + 2] - 'a';
        int toR = 7 - (str[i + 3] - '1');

        int fromSq = fromR * 8 + fromF;
        int toSq = toR * 8 + toF;

        chooseMove(fromSq, toSq);
    }
}
