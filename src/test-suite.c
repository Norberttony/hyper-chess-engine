
#include "test-suite.h"

#define NUM_TEST_SUITE_POSITIONS 20

// shorthand
typedef struct TestSuitePosition pos;

pos positions[NUM_TEST_SUITE_POSITIONS] = {
    {
        "Starting position",
        StartingFEN,
        {1, 32, 944, 42738},
        {0,  0,   0,   162},
        {0,  0,   0,   162},
        {0,  0,   0,     0}
    },
    {
        // the FEN of this position is courtesy of
        // https://carusos.org/Hyperchess/hyperchess.html
        "Chameleon-coordinator checkmate",
        "k5R1/p7/8/8/7K/8/8/5B2 w -",
        {1, 46, 608},
        {0,  0,   1},
        {0,  0,   1},
        {0,  3,   3}
    },
    {
        "White king-chameleon death squares with king move",
        "unbqkbn1/pppppppp/3PPP2/3r1B2/8/8/PPP3PP/RNBQK1NU w 8",
        {1, 63, 2067, 126668},
        {0,  1,   83,   2549},
        {0,  1,   83,   2549},
        {0,  0,    0,      0}
    },
    {
        "Black king-chameleon death squares with king move",
        "un1q1kb1/pp1ppppn/P2PPP1p/4RB1b/8/2p4P/1PPK2P1/1NBQ2NU b 16",
        {1, 41, 2889, 119445},
        {0,  2,   52,   6145},
        {0,  2,   52,   6146},
        {0,  0,    0,      0}
    },
    {
        "straddler custodial capture only after move made",
        "p1P4k/4p3/Pp2P2p/3Pp3/p1Pp4/1Pp2P2/P2u4/3Pp2K w -",
        {1, 47, 2850, 133921},
        {0,  8,  223,  19042},
        {0, 10,  227,  22491},
        {0,  0,    0,      0}
    },
    {
        "straddler and chameleon duo",
        "p1PP1P1k/4pp2/Pp2P2p/3Pp3/p1Pq1P2/1Bp2B2/P2p4/3Pp1pK w -",
        {1, 56, 3554, 207069},
        {0, 13,  240,  41026},
        {0, 15,  242,  45830},
        {0,  0,    0,      0}
    },
    {
        "straddlers only work with other friendly straddlers",
        "p1P4k/4p2N/Ppp1P2p/3Pp3/p1Pp4/1Pp2P1P/P2u4/K2Pp2N w -",
        {1, 58, 3251, 189823},
        {0,  7,  402,  21447},
        {0,  9,  453,  26199},
        {0,  0,    0,      0}
    },
    {
        "straddler-chameleon duo and co (cannot work with them though)",
        "p1PP1P1k/4pp1N/Pp2P2p/3Pp3/p1Pq1P2/1Bp2b1B/P2p4/3Pp1pK w -",
        {1, 61, 4277, 261371},
        {0, 12,  423,  47311},
        {0, 13,  424,  51002},
        {0,  0,   56,     62}
    },
    {
        "springer test 1",
        "4p2k/1b2p3/6p1/5p2/1p1pN2p/8/2n1P3/K6n w -",
        {1, 20, 2121, 53013},
        {0,  3,   64,  5041},
        {0,  3,   64,  5041},
        {0,  0,    0,     0}
    },
    {
        "chameleon-springer test",
        "4p2k/1b2p3/6p1/5p2/1p1pB2p/8/2n1P3/K6n w -",
        {1, 18, 1905, 43518},
        {0,  1,   48,   939},
        {0,  1,   48,   939},
        {0,  0,    0,     0}
    },
    {
        "king and coordinator basics",
        "Pk5p/r7/8/8/p4R2/8/8/K4n1p w -",
        {1, 23, 1258, 30237},
        {0, 16,  104,  7225},
        {0, 18,  107,  7908},
        {0,  0,    0,     0}
    },
    {
        "king and coordinator basics",
        "Pk5p/r7/8/8/p4R2/8/8/K4n1p w -",
        {1, 23, 1258, 30237},
        {0, 16,  104,  7225},
        {0, 18,  107,  7908},
        {0,  0,    0,     0}
    },
    {
        "king-chameleon duo",
        "Pk5p/r6B/8/8/p4B2/8/8/K4n1p w -",
        {1, 41, 2064, 81600},
        {0,  9,  198, 10267},
        {0,  9,  198, 10287},
        {0,  0,    0,     1}
    },
    {
        "coordinator-chameleon duo",
        "R5k1/8/8/B7/p3B3/8/8/K4n1P w -",
        {1, 60, 1426, 91295},
        {0,  1,   17,  2394},
        {0,  1,   17,  2432},
        {0,  1,    1,   846}
    },
    {
        "immobilizer checkmates",
        "N4B1B/8/4k3/3u1U2/4K3/8/8/8 w -",
        {1, 55, 784, 48377},
        {0,  0,   0,   164},
        {0,  0,   0,   164},
        {0,  1,   1,  1171}
    },
    {
        "immobilized immobilizer",
        "N4B1B/8/4k3/4uU2/4K3/8/8/8 w -",
        {1, 38, 38, 38},
        {0,  0,  0,  0},
        {0,  0,  0,  0},
        {0,  5,  5,  5}
    },
    {
        "chameleoned immobilizer",
        "N4B1B/8/4k3/4bU2/4K3/8/8/8 w -",
        {1, 45, 45, 45},
        {0,  1,  1,  1},
        {0,  1,  1,  1},
        {0, 11, 11, 11}
    },
    {
        "retractor",
        "qP5k/PP6/8/3p1P2/3pQp2/3p4/8/7K w -",
        {1, 41, 1187, 57057},
        {0,  1,    6,  1340},
        {0,  1,    6,  1340},
        {0,  0,    0,     0}
    },
    {
        "chameleon retractor",
        "pp5k/pp6/8/p1qp1QP1/PB1pBp2/3p4/8/7K w -",
        {1, 44, 2183, 107986},
        {0,  4,   85,   6475},
        {0,  4,   85,   6477},
        {0,  0,    0,      0}
    },
    {
        "one move. five pieces. all captured.",
        "k3r2K/8/8/4P3/4p3/2Pp1pP1/4B3/4q3 w -",
        {1, 6, 246, 8750},
        {0, 2,   2,  701},
        {0, 6,   6, 1229},
        {0, 0,   0,    0}
    }
};

int runTestSuite()
{
    int nodes = 0; // so yeah. it's not "move count." it's the number of leaf nodes. oops.

    int fullMatch = 1;

    puts("Running test suite...");
    clock_t start = clock();
    for (int i = 0; i < NUM_TEST_SUITE_POSITIONS; i++)
    {
        puts("---------------------------------------------");
        pos *pos = &positions[i];

        printf("Using test position %s\n", pos->name);
        printf("FEN: %s\n", pos->fen);

        loadFEN(pos->fen);
        prettyPrintBoard();

        int depth = 0;
        while (pos->moveCounts[depth] != 0)
        {
            printf("===Counting at depth %d===\n", depth);
            struct MoveCounter count = countMoves(depth);

            nodes += count.moves;

            int match = 1;

            // move count
            puts("");
            printf("Move count: %d\n", count.moves);
            printf("Expected: %d\n", pos->moveCounts[depth]);
            if (count.moves != pos->moveCounts[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // capture moves
            puts("");
            printf("Capture moves count: %d\n", count.captureMoves);
            printf("Expected: %d\n", pos->captureMoves[depth]);
            if (count.captureMoves != pos->captureMoves[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // pieces captured
            puts("");
            printf("Pieces captured count: %d\n", count.pieceCaptures);
            printf("Expected: %d\n", pos->pieceCaptures[depth]);
            if (count.pieceCaptures != pos->pieceCaptures[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }

            // checkmates
            puts("");
            printf("Checkmates count: %d\n", count.checkmates);
            printf("Expected: %d\n", pos->checkmates[depth]);
            if (count.checkmates != pos->checkmates[depth])
            {
                puts("NO MATCH!");
                match = 0;
            }
            
            puts("");
            if (!match)
            {
                puts("ERROR HERE <------------------------------------------------------------------------------");
                fullMatch = 0;
            }

            depth++;
        }
    }
    puts("---------------------------------------------");
    clock_t end = clock();

    puts("Test suite complete!");
    if (!fullMatch)
    {
        puts("Some results did not match!");
    }
    else
    {
        puts("All test suite positions succeeded!");
    }
    printf("Took %f seconds. Explored %d nodes.\n", (float)(end - start) / CLOCKS_PER_SEC, nodes);
}
