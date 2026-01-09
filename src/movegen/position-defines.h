#ifndef POSITION_DEFINES_HEADER
#define POSITION_DEFINES_HEADER

#include <stdint.h>
#include "bitboard-utility.h"
#include "move.h"

#define get_zobrist_hash(sq, type, isWhite) zobristHashes[64 * type + sq + 64 * 7 * !isWhite]

// determines "x-move-rule" which is "draw in x noncapturing moves"
#define DRAW_MOVE_RULE 100

// 30 capturable pieces that each extend the fifty move rule by 100 additional halfmoves.
#define MAX_GAME_LENGTH (DRAW_MOVE_RULE * 30)

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

// whose side it is to play
enum
{
    white = 0, black = 8
};

typedef struct PositionState
{
    int halfmove;
    // the move that was played to lead into this position
    Move prevMove;
} PositionState;

typedef struct Position
{
    U64 boards[16];
    int* pieceList;
    int toPlay;
    int notToPlay;
    int fullmove;
    // [0] accesses white's material score and [1] accesses black's material score.
    int materialScore[2];
    U64 zobristHash;
    PositionState* state;
} Position;

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

// returns the number of times the current position has been repeated.
int getNumberOfRepeats(void);

// used for when the repeat table might be cleared out when printing out the PV.
void saveRepeatTable(void);
void restoreRepeatTable(void);

static inline Move get_move_n_ply_ago(int ply)
{
    if (g_pos.state - g_states >= ply)
    {
        return (g_pos.state - ply + 1)->prevMove;
    }
    return 0;
}

#endif
