
#include "test-suite.h"

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
        {1, 41, 2889, 119455},
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
    },

    // For testing absolute pins
    {
        "chameleon pin",
        "pnp3k1/1B6/8/8/8/8/6R1/7K b -",
        {1, 2, 62, 1885, 60568},
        {0, 1,  1,   41,   622},
        {0, 1,  1,   41,   622},
        {0, 0,  0,    0,    21}
    },
    {
        "Springer pin 1",
        "8/8/N3pk2/8/8/8/8/7K b -",
        {1, 10, 204, 3063, 73275},
        {0,  0,  10,   20,  1192},
        {0,  0,  10,   20,  1192},
        {0,  0,   0,    0,     0}
    },
    {
        "Springer pin 2",
        "8/8/N3pkp1/8/8/8/8/7K b -",
        {1, 24, 489, 10765, 254955},
        {0,  0,  12,    30,   6305},
        {0,  0,  12,    30,   6305},
        {0,  0,   0,     0,      0}
    },
    {
		"Springer pin 3",
		"8/8/N3pk2/8/8/8/8/7K b -",
		{1, 10, 204, 3063, 73275},
		{0, 0, 10, 20, 1192},
		{0, 0, 10, 20, 1192},
		{0, 0, 0, 0, 0}
	},
	{
		"Coordinator pin",
		"1p4k1/8/8/8/8/8/8/1R4K1 b -",
		{1, 10, 208, 2099, 51718},
		{0, 0, 3, 12, 416},
		{0, 0, 3, 12, 416},
		{0, 0, 0, 0, 2}
	},
	{
		"Retractor pin 1",
		"8/8/4b3/4Q3/4k3/8/8/7K b -",
		{1, 5, 83, 2133, 47441},
		{0, 2, 5, 38, 348},
		{0, 2, 5, 38, 348},
		{0, 0, 0, 0, 0}
	},
	{
		"Retractor pin 2",
		"8/8/4p3/4Q3/4k3/8/8/7K b -",
		{1, 4, 80, 1333, 30751},
		{0, 1, 4, 17, 244},
		{0, 1, 4, 17, 244},
		{0, 0, 0, 0, 0}
	},
	{
		"Straddler pin 1",
		"k7/8/3pKBp1/8/8/8/8/8 w -",
		{1, 7, 136, 3362, 83000},
		{0, 1, 7, 95, 343},
		{0, 1, 7, 95, 343},
		{0, 0, 0, 0, 0}
	},
	{
		"Straddler pin 2",
		"k7/8/1pK2Bp1/8/8/8/8/8 w -",
		{1, 9, 171, 4564, 110304},
		{0, 1, 6, 70, 322},
		{0, 1, 6, 70, 322},
		{0, 0, 0, 0, 0}
	},
	{
		"Straddler pin 3",
		"k7/8/3pKB2/8/8/5p2/8/8 w -",
		{1, 9, 201, 4880, 124893},
		{0, 1, 1, 101, 195},
		{0, 1, 1, 101, 195},
		{0, 0, 0, 0, 0}
	},
    {
		"Straddler pin 4",
		"k4p2/8/3pKB2/8/8/5p2/8/8 w -",
		{1, 6, 174, 3497, 116607},
		{0, 1, 1, 132, 495},
		{0, 1, 1, 132, 495},
		{0, 0, 0, 0, 0}
	},
    {
		"Straddler pins springer 1",
		"k4p2/8/3pKN2/8/8/5p2/8/8 w -",
		{1, 6, 174, 3703, 120280},
		{0, 1, 1, 338, 704},
		{0, 1, 1, 338, 704},
		{0, 0, 0, 0, 0}
	},
	{
		"Straddler pins springer 2",
		"k7/8/3pKN2/8/8/5p2/8/8 w -",
		{1, 10, 214, 5518, 132436},
		{0, 2, 2, 353, 466},
		{0, 2, 2, 353, 466},
		{0, 0, 0, 0, 0}
	},
	{
		"King pin 1",
		"ppp3k1/1K6/8/8/8/8/6R1/8 b -",
		{1, 2, 58, 851, 20686},
		{0, 0, 8, 12, 2511},
		{0, 0, 8, 12, 2671},
		{0, 0, 0, 0, 40}
	},

    // For absolute pins that have to deal with the immobilizer
    {
		"Immobilizer chameleon pin",
		"pnp3k1/1B6/1u6/8/8/8/6R1/7K b -",
		{1, 25, 515, 16254, 351379},
		{0, 0, 10, 164, 7140},
		{0, 0, 10, 164, 7157},
		{0, 0, 0, 0, 92}
	},
	{
		"Immobilizer springer pin 1",
		"8/8/N3pk2/u7/8/8/8/7K b -",
		{1, 35, 370, 11537, 222516},
		{0, 0, 3, 3, 4799},
		{0, 0, 3, 3, 4799},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer springer pin 2",
		"8/8/N3pkp1/u7/8/8/8/7K b -",
		{1, 42, 391, 15455, 291262},
		{0, 0, 5, 5, 7077},
		{0, 0, 5, 5, 7077},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer springer pin 3",
		"8/8/N5pk/u7/8/8/8/7K b -",
		{1, 34, 399, 14230, 298364},
		{0, 0, 5, 5, 5197},
		{0, 0, 5, 5, 5197},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer coordinator pin 1",
		"1p4k1/8/8/8/8/8/8/uR4K1 b -",
		{1, 30, 366, 6951, 116948},
		{0, 0, 8, 20, 1948},
		{0, 0, 8, 20, 1948},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer retractor pin 1",
		"8/8/4b3/4Qu2/4k3/8/8/7K b -",
		{1, 37, 327, 11452, 204364},
		{0, 2, 2, 250, 1398},
		{0, 2, 2, 250, 1398},
		{0, 0, 0, 3, 3}
	},
	{
		"Immobilizer retractor pin 2",
		"8/8/4p3/4Qu2/4k3/8/8/7K b -",
		{1, 29, 303, 8339, 158280},
		{0, 1, 1, 120, 1052},
		{0, 1, 1, 120, 1052},
		{0, 0, 0, 3, 3}
	},
	{
		"Immobilizer straddler pin 1",
		"k7/8/3pKBp1/4U3/8/8/8/8 w -",
		{1, 24, 393, 15711, 320879},
		{0, 1, 1, 247, 1128},
		{0, 1, 1, 247, 1128},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer straddler pin 2",
		"k7/8/1pK2Bp1/1U6/8/8/8/8 w -",
		{1, 25, 333, 14342, 270827},
		{0, 1, 1, 183, 763},
		{0, 1, 1, 183, 763},
		{0, 0, 0, 2, 2}
	},
	{
		"Immobilizer straddler pin 3",
		"k7/8/3pKB2/4U3/8/5p2/8/8 w -",
		{1, 26, 457, 17528, 355251},
		{0, 1, 2, 307, 867},
		{0, 1, 2, 307, 867},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer straddler pin 4",
		"k4p2/8/3pKB2/4U3/8/5p2/8/8 w -",
		{1, 23, 578, 18518, 511698},
		{0, 1, 2, 459, 2375},
		{0, 1, 2, 459, 2376},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer straddler pin 5",
		"k7/5p2/3pKBU1/5p2/8/5p2/8/8 w -",
		{1, 26, 673, 17576, 501595},
		{0, 3, 5, 1471, 5296},
		{0, 3, 5, 1471, 5314},
		{0, 0, 0, 0, 0}
	},
	{
		"Immobilizer king pin",
		"ppp3k1/1K6/1u6/8/8/8/6R1/8 b -",
		{1, 23, 416, 14371, 296406},
		{0, 0, 38, 62, 40039},
		{0, 0, 39, 63, 41846},
		{0, 0, 0, 42, 52}
	},
    {
		"Retractor-coordinator double pin",
		"6R1/8/4b3/4Q3/4k1p1/8/8/4K3 b -",
		{1, 4, 151, 3281, 121670},
		{0, 1, 4, 99, 2056},
		{0, 1, 4, 100, 2057},
		{0, 0, 0, 2, 2}
	},
    {
		"Coordinator deals with retractor pin",
		"8/8/4r3/4Q3/4k3/7K/8/8 b -",
		{1, 6, 212, 3682, 135249},
		{0, 0, 7, 107, 4620},
		{0, 0, 7, 108, 4666},
		{0, 0, 1, 3, 153}
	},
    {
		"Straddler retractor pin",
		"k7/8/3pKQ2/5p2/8/8/8/8 w -",
		{1, 7, 130, 3199, 72219},
		{0, 3, 3, 111, 209},
		{0, 3, 3, 111, 209},
		{0, 0, 0, 0, 0}
	},
    {
		"Straddler and immobilizer pin",
		"k7/8/3pK1U1/5p2/8/5p2/8/8 w -",
		{1, 10, 214, 4179, 111926},
		{0, 2, 2, 171, 221},
		{0, 2, 2, 171, 221},
		{0, 0, 0, 0, 0}
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
