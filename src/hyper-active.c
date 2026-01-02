#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WEB
#include <emscripten.h>
#endif

// move generation
#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"
#include "move.h"
#include "perft.h"
#include "uci.h"

// initializes the move generator
void initMoveGen(void);


#ifdef WEB
EMSCRIPTEN_KEEPALIVE
#endif
int main(void)
{
    initMoveGen();

    srand(time(NULL));

    loadFEN(StartingFEN);

    uciLoop();

    puts("Good game!");

    return 0;
}

void initMoveGen(void)
{
    // Initialization!
    generateZobristHashes();
    populateKingMoves();
    populateRanksAndFiles(); // in order to use genDeathSquares (used by populateDeathSquares)
    populateDeathSquares();
    populateSpringerLeaps();
    populateSpringerCaptures();
    populateRetractorCaptures();
    initMagicBitboards(0); // rook magic bitboards
    initMagicBitboards(1); // bishop magic bitboards
    initPSQT();
}
