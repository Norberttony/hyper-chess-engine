
#include <stdio.h>

#include <time.h>

#include "bitboard-utility.h"
#include "magic-bitboards.h"
#include "look-up-tables.h"
#include "defines.h"
#include "move.h"
#include "perft.h"


int main(void)
{
    populateKingMoves();
    populateRanksAndFiles(); // in order to use genDeathSquares (used by populateDeathSquares)
    populateDeathSquares();
    populateSpringerLeaps();
    populateSpringerCaptures();
    populateRetractorCaptures();
    initMagicBitboards(0); // rook magic bitboards
    initMagicBitboards(1); // bishop magic bitboards

    // initial board set up
    loadFEN(StartingFEN);

    // print position
    prettyPrintBoard();

    loadFEN("1Bu5/1pP5/8/3b4/5U2/8/8/8 w -");
    puts("Test position:");
    prettyPrintBoard();
    printPieceList();

    int depth = 3;

    clock_t start = clock();
    struct MoveCounter counter = divide(depth);
    clock_t end = clock();

    printf("At depth %d there are %d moves and %d of them capture something. Among all captures, %d pieces are captured.\n", depth, counter.moves, counter.captureMoves, counter.pieceCaptures);
    printf("Time taken: %f\n", (float)(end - start) / CLOCKS_PER_SEC);

    prettyPrintBoard();
    printPieceList();

    //getchar();

    return 0;
}
