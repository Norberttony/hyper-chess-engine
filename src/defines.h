#ifndef DEFINES_HEADER
#define DEFINES_HEADER

#include <ctype.h> // for tolower and toupper
#include <stdlib.h>

#include "bitboard-utility.h"

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

extern int toPlay;
extern int notToPlay;
extern int fullmove;

extern U64 position[17];

extern const char pieceFEN[];

extern const char StartingFEN[];

extern int pieceList[64];

// 14 pieces, 64 squares, which side to play
#define ZOBRIST_HASH_COUNT (64 * 14 + 1)

extern U64 zobristHashes[ZOBRIST_HASH_COUNT];
extern U64 zobristHash;

#define REPEAT_TABLE_ENTRIES 32
extern U64 repeatTable[REPEAT_TABLE_ENTRIES];
extern int repeatTableIndex;

// prints all positions onto one board with FEN symbols
void prettyPrintBoard();

// expects fen to be three space-separated items:
// - the FEN of the position
// - whose side it is to play
// - the fullmove counter, '-' if none
// this function does not check for legal FENs nor does it try to correct unnecessary spaces.
// returns 1 on success and 0 on failure
int loadFEN(const char* fen);

// converts the given piece FEN to its corresponding value
int convertFENToValue(const char v);

void printPieceList();

// generates zobrist hashes
void generateZobristHashes();

#endif