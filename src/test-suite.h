#ifndef TEST_SUITE_HEADER
#define TEST_SUITE_HEADER

#include <stdio.h>
#include <time.h>

#include "defines.h"
#include "perft.h"

#define NUM_TEST_SUITE_POSITIONS 31

struct TestSuitePosition
{
    const char* name;
    const char* fen;

    // these 4 have an index of depth
    int moveCounts[20];
    int captureMoves[20];
    int pieceCaptures[20];
    int checkmates[20];
};

// runs all of the positions in the test suite.
// returns 0 on failure and 1 on success
int runTestSuite();

#endif