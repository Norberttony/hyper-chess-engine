#ifndef SEARCH_DEFINES_HEADER
#define SEARCH_DEFINES_HEADER

#include <stdint.h>
#include "bitboard-utility.h"
#include "move.h"

// defines the maximum possible number of moves in any attainable position
#define MAX_MOVES 256
#define MAX_CAPTURES 90

// defines the max depth that the engine can calculate to.
#define MAX_DEPTH 127

typedef uint_fast8_t SearchFlags;

typedef struct SearchParams
{
    int thinkingTime;
    int thinkStart;
    int maxDepth;
} SearchParams;

typedef struct SearchResults
{
    int thinkingTime;
    int thinkStart;
    int stopThinking;
    int height;
    U64 nodesVisited;
    Move bestMove;
} SearchResults;

// used for storing mate in x evaluations. extract_mate_scores will return the depth until mate.
#define MAX_SCORE 4194303
#define MATE_SCORE (MAX_SCORE - 10000)
#define extract_mate_score(score) (MAX_SCORE - score)

#endif
