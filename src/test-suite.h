#ifndef TEST_SUITE_HEADER
#define TEST_SUITE_HEADER

#include <stdio.h>
#include <time.h>
#include "defines.h"
#include "perft.h"

#define NUM_TEST_SUITE_POSITIONS 49

struct TestSuitePosition
{
    const char* name;
    const char* fen;

    // these 4 have an index of depth
    U64 moveCounts[20];
    U64 captureMoves[20];
    U64 pieceCaptures[20];
    U64 checkmates[20];
};

// shorthand
typedef struct TestSuitePosition pos;

// runs all of the positions in the test suite.
// returns 0 on failure and 1 on success
int runTestSuite(void);

#endif
