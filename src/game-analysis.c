
#include "game-analysis.h"


void playGame(int mySide)
{
    // time to play the game!
    while (1)
    {
        puts("");
        if (checkGameOver())
        {
            break;
        }

        // time to consider which moves are played
        if (mySide == g_pos.toPlay)
        {
            Move best = thinkFor(200);

            fflush(stdout);
            makeMove(best);
            prettyPrintBoard();
        }
        else
        {
            puts("Your turn! Type in a move:");
            
            int* squares = askForMove();

            // try to play the move...
            if (!chooseMove(squares[0], squares[1]))
            {
                puts("That's not a legal move");
            }
            else
            {
                prettyPrintBoard();
            }

            free(squares);
        }
    }
}

void askForFEN()
{
    // prompt user for FEN
    puts("Paste in the FEN:");
    fflush(stdout);
    
    char chosenFEN[1000];
    fgets(chosenFEN, 1000, stdin);

    // load user's choice, or if that fails, the starting FEN
    if (!loadFEN(chosenFEN))
    {
        puts("Loading starting FEN instead...");
        loadFEN(StartingFEN);
    }
}

int askForSide()
{
    // prompt user for which side they will play
    puts("Which side do you want to play? (w/b)");
    fflush(stdout);

    char sideInput = 0;
    do
    {
        sideInput = getchar();
        scanf("%*c"); // ignore newline character

        if (sideInput != 'w' && sideInput != 'b')
        {
            puts("That's not a valid side, choose either white (w) or black (b)");
        }
    } while (sideInput != 'w' && sideInput != 'b');

    return sideInput == 'w' ? black : white;
}

int* askForMove()
{
    int* squares = (int*) malloc(2 * sizeof(int));

    char move[10]; // yes, risks buffer overflow, I know...
    scanf("%s", (char*)&move);

    // get coordinates of from square and to square
    int fromF = move[0] - 'a';
    int fromR = 7 - (move[1] - '1');

    int toF = move[2] - 'a';
    int toR = 7 - (move[3] - '1');

    squares[0] = fromR * 8 + fromF;
    squares[1] = toR * 8 + toF;

    return squares;
}

void analyzeGame(int depth)
{
    while (1)
    {
        if (checkGameOver())
            break;

        // perform iterative deepening
        for (int i = 1; i < depth; i++)
        {
            orderFirst = getBestMove(i);
        }
        Move m = getBestMove(depth);

        printEval();
        printPrincipalVariation(depth);

        puts("Phil certified recommendation: ");
        prettyPrintMove(m);
        puts("");


        puts("Type in a move");
        int* squares = askForMove();

        // try to play the move...
        if (!chooseMove(squares[0], squares[1]))
        {
            puts("That's not a legal move");
        }

        free(squares);
    }
}

int checkGameOver()
{
    // yeah, a bit of a bad way to check for game-ending situations. oh well...
    Move movelist[MAX_MOVES];
    int size = generateMoves((Move*)movelist, 0);
    // consider only legal moves
    int legalMoves = 0;
    for (int i = 0; i < size; i++)
    {
        legalMoves += isMoveLegal(movelist[i]);
    }
    if (legalMoves == 0)
    {
        if (isCheckmate())
        {
            if (g_pos.toPlay == white)
            {
                puts("result 0-1");
            }
            else
            {
                puts("result 1-0");
            }
        }
        else
        {
            puts("result 1/2-1/2");
        }
        fflush(stdout);
        return 1;
    }

    return 0;
}

void askForLAN(void)
{
    puts("Paste in the LAN");
    fflush(stdout);

    char lan[10000];
    fgets(lan, 10000, stdin);

    loadLAN(lan);
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
