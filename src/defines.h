#ifndef DEFINES_HEADER
#define DEFINES_HEADER

// for tolower and toupper
#include <ctype.h>

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
extern int fullmove;

extern U64 position[17];

extern const char pieceFEN[];

extern const char StartingFEN[];

// prints all positions onto one board with FEN symbols
void prettyPrintBoard();

// expects fen to be three space-separated items:
// - the FEN of the position
// - whose side it is to play
// - the fullmove counter, '-' if none
// this function does not check for legal FENs nor does it try to correct unnecessary spaces.
void loadFEN(const char* fen);

// converts the given piece FEN to its corresponding value
int convertFENToValue(const char v);

#endif