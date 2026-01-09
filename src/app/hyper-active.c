#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WEB
#include <emscripten.h>
#endif

// move generation
#include "../movegen/magic-bitboards.h"
#include "../movegen/move.h"
#include "../search/perft.h"
#include "../uci/uci.h"
#include "../movegen/look-up-tables.h"
#include "../eval/evaluate-defines.h"

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
