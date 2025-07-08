#ifndef DEFINES_HEADER
#define DEFINES_HEADER

#include <ctype.h> // for tolower and toupper
#include <stdlib.h>

#include "evaluate-defines.h"
#include "bitboard-utility.h"


// determines "x-move-rule" which is "draw in x noncapturing moves"
#define DRAW_MOVE_RULE 100

// 30 capturable pieces that each extend the fifty move rule by 100 additional halfmoves.
#define MAX_GAME_LENGTH (DRAW_MOVE_RULE * 30)
#define get_zobrist_hash(sq, type, isWhite) zobristHashes[64 * type + sq + 64 * 7 * !isWhite]

// whose side it is to play
enum
{
    white = 0, black = 8
};

// piece types
enum
{
    _,
    straddler,
    retractor,
    springer,
    coordinator,
    immobilizer,
    chameleon,
    king
};

typedef struct PositionState
{
    int halfmove;
} PositionState;

typedef struct Position
{
    U64 boards[16];
    int* pieceList;
    int toPlay;
    int notToPlay;
    int fullmove;
    int materialScore[2];
    U64 zobristHash;
    PositionState* state;
} Position;

typedef struct SearchParams
{
    int thinkingTime;
    int thinkStart;
    int stopThinking;
    int maxDepth;
    int height;
} SearchParams;

extern Position g_pos;

extern PositionState g_states[MAX_GAME_LENGTH];

extern const char pieceFEN[];

extern const char StartingFEN[];

extern const char* squareNames[];

// 7 pieces on each side (so 14) on any of the 64 squares, which side to play
// the additional 64 squares at the beginning are to not have to make bounds check when searching
// for the zobrist hash of a particular piece.
#define ZOBRIST_HASH_COUNT (64 + 64 * 14 + 1)
#define ZOBRIST_HASH_COUNT_HALFMOVE 20


extern U64 zobristHashes[ZOBRIST_HASH_COUNT];
extern U64 zobristHashes_halfmoves[ZOBRIST_HASH_COUNT_HALFMOVE + 1];

#define REPEAT_TABLE_ENTRIES 32
extern U64 repeatTable[REPEAT_TABLE_ENTRIES];
extern int repeatTableIndex;

// used for storing mate in x evaluations. extract_mate_scores will return the depth until mate.
#define MAX_SCORE 4194303
#define MATE_SCORE (MAX_SCORE - 10000)
#define extract_mate_score(score) (MAX_SCORE - score)


// prints all positions onto one board with FEN symbols
void prettyPrintBoard(void);

// expects fen to be three space-separated items:
// - the FEN of the position
// - whose side it is to play
// - the fullmove counter, '-' if none
// this function does not check for legal FENs nor does it try to correct unnecessary spaces.
// returns 1 on success and 0 on failure
int loadFEN(const char* fen);

// returns the FEN
int getFEN(char* fen, int bufsize);

// converts the given piece FEN to its corresponding value
int convertFENToValue(const char v);

void printPieceList(void);

// generates zobrist hashes
void generateZobristHashes(void);

// returns 1 if the threefold flag is activated and 0 otherwise
int getThreefoldFlag(void);

#endif
