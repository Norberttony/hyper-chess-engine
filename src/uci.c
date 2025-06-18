
#include "uci.h"

#define INPUT_BUFFER 5000


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

#ifdef WEB
        // Allow browser to catch up
        emscripten_sleep(100);
#endif

        // according to UCI, every command must end with a line break.
        if (!fgets(line, INPUT_BUFFER, inputFile) || line[0] == '\n')
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
            stopThinking = -1;
            break;
        }
        else if (!strncmp(line, "uci", 3))
        {
            uciOk();
        }
        else if (!strncmp(line, "clear hash", 10))
        {
            memset(transpositionTable, 0, sizeof(transpositionTable));
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
        if (stopThinking == -1)
        {
            break;
        }
    }

    if (inputFile != stdin)
    {
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
    int movesToGo = 30;
    int moveTime = -1;
    int time = -1;
    int inc = 0;

    // match various parameters given by the user
    char* matchAt = NULL;

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

    // interpret the commands

    // think infinitely by default
    thinkStart = getCurrentTime();
    thinkingTime = -1;

    if (moveTime != -1)
    {
        time = moveTime;
        movesToGo = 1;
    }

    if (time != -1)
    {
        time /= movesToGo;
        time -= 50; // some buffer to prevent bot from timing out
        if (time < 0)
        {
            time = 0;
        }
        thinkingTime = time + inc;
    }

    if (depth == -1)
    {
        maxDepth = MAX_DEPTH;
    }
    else
    {
        maxDepth = depth;
    }

    startThink();
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
            if (spaces == 3)
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

void readInput(void)
{
#ifndef WEB
    int bytes;

    char input[256] = "", *endc;

    if (inputIsWaiting())
    {
        puts("info string Paused. Type in 'stop'/'quit' to stop or just press Enter to continue.");
        do
        {
            bytes = read(STDIN_FILENO, input, 256);
        }
        while (bytes <= 0);
        puts("info string Input received.");

        endc = strchr(input, '\n');

        if (endc)
        {
            *endc = 0;
        }

        // if input is available
        if (strlen(input) > 0)
        {
            if (!strncmp(input, "quit", 4) || !strncmp(input, "stop", 4))
            {
                stopThinking = 1;
            }
        }
    }
#endif
#ifdef WEB
    // Allow any browser events to catch up and handle I/O
    static int emscriptenSleepCounts = 0;
    if (emscriptenSleepCounts++ >= 1000)
    {
        emscripten_sleep(1);
        emscriptenSleepCounts = 0;
    }
#endif
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
