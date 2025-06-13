
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <emscripten.h>

// move generation
#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"
#include "move.h"
#include "perft.h"
#include "test-suite.h"

#include "game-analysis.h"

// initializes the move generator
void initMoveGen(void);

EMSCRIPTEN_KEEPALIVE
int main(void)
{
    initMoveGen();
    loadFEN(StartingFEN);

    puts("Hyper Active is ready to play.");

    return 0;
}

EMSCRIPTEN_KEEPALIVE
Move thinkForWasm(int time)
{
    return thinkFor(time);
}

EMSCRIPTEN_KEEPALIVE
void chooseMoveWasm(int from, int to)
{
    chooseMove(from, to);
}

EMSCRIPTEN_KEEPALIVE
void loadFENWasm(char* fen)
{
    loadFEN(fen);
}

EMSCRIPTEN_KEEPALIVE
void setTTSizeWasmInMB(int mb)
{
    setTranspositionTableSize(mb);
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
