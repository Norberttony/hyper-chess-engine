
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
    initMagicBitboards(0); // rook magic bitboards
    initMagicBitboards(1); // bishop magic bitboards

    // initial board set up
    loadFEN(StartingFEN);

    // print position
    prettyPrintBoard();

    // play a move on the board!!
    struct MoveList* moveList = generateMoves();
    Move chosen = moveList->list[10];
    puts("Playing move:");
    prettyPrintMove(chosen);
    makeMove(chosen);
    free(moveList); // must now free memory

    prettyPrintBoard();

    moveList = generateMoves();
    chosen = moveList->list[6];
    puts("Playing move:");
    prettyPrintMove(chosen);
    makeMove(chosen);
    free(moveList);

    prettyPrintBoard();

    printBitboard(position[black]);
    moveList = generateMoves();
    chosen = moveList->list[11];
    puts("Playing move:");
    prettyPrintMove(chosen);
    makeMove(chosen);
    free(moveList);

    prettyPrintBoard();

    printPieceList();

    puts("Undoing the last move...");
    unmakeMove(chosen);

    prettyPrintBoard();

    printPieceList();

    loadFEN("p1P5/4p3/Pp2P2p/3Pp3/p1Pp4/1Pp2P2/P2p4/3Pp3 w -");
    puts("Test position:");
    prettyPrintBoard();

    int depth = 5;

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
