#ifndef DEFINES_HEADER
#define DEFINES_HEADER

#include <ctype.h> // for tolower and toupper
#include <stdlib.h>

#include "bitboard-utility.h"

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

extern int toPlay;
extern int notToPlay;
extern int halfmove;

extern U64 position[17];

extern const char pieceFEN[];

extern const char StartingFEN[];

// 8 squares above and 8 squares below as extra padding
extern int pieceListStore[80];
extern int* pieceList;

// 7 pieces on each side (so 14) on any of the 64 squares, which side to play
// the additional 64 squares at the beginning are to not have to make bounds check when searching
// for the zobrist hash of a particular piece.
#define ZOBRIST_HASH_COUNT (64 + 64 * 14 + 1)

extern U64 zobristHashes[ZOBRIST_HASH_COUNT];
extern U64 zobristHash;

#define REPEAT_TABLE_ENTRIES 32
extern U64 repeatTable[REPEAT_TABLE_ENTRIES];
extern int repeatTableIndex;

// used for storing mate in x evaluations. extract_mate_scores will return the depth until mate.
#define MATE_SCORE (INT_MAX - 10000)
#define extract_mate_score(score) (10000 - (score - MATE_SCORE))


// prints all positions onto one board with FEN symbols
void prettyPrintBoard();

// expects fen to be three space-separated items:
// - the FEN of the position
// - whose side it is to play
// - the fullmove counter, '-' if none
// this function does not check for legal FENs nor does it try to correct unnecessary spaces.
// returns 1 on success and 0 on failure
int loadFEN(const char* fen);

// returns the FEN
char* getFEN();

// converts the given piece FEN to its corresponding value
int convertFENToValue(const char v);

void printPieceList();

// generates zobrist hashes
void generateZobristHashes();

// returns 1 if the threefold flag is activated and 0 otherwise
int getThreefoldFlag();

#endif