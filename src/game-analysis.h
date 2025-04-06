#ifndef GAME_ANALYSIS_H
#define GAME_ANALYSIS_H


#include <stdio.h>
#include <time.h>

#include "move.h"
#include "move-ordering.h"

#include "perft.h"

// algorithm
#include "evaluate.h"
#include "transposition-table.h"
#include "think.h"


// asks user for FEN and sets it. if the FEN is invalid, the starting FEN is loaded instead.
void askForFEN(void);

// returns the side the user wants to play as.
int askForSide(void);

// returns a 2 element array containing the from and to squares, respectively.
// this element must be freed after use
int* askForMove(void);

// keeps playing moves until the game is over.
// does not check for threefold repetition or fifty move rule.
void playGame(int engineSide);

// puts the program into an analysis mode where the engine thinks for the fixed depth
void analyzeGame(int depth);

int checkGameOver(void);

void askForLAN(void);

void loadLAN(char* lan);

#endif
