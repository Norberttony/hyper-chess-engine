
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
void initMoveGen();


int main(void)
{
    initMoveGen();

    srand(time(NULL));

    askForFEN();

    //askForLAN();

    int mySide = askForSide();

    prettyPrintBoard();

    playGame(mySide);

    puts("Good game!");

    return 0;
}

void initMoveGen()
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
}
