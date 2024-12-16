#ifndef UCI_HEADER
#define UCI_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "think.h"
#include "defines.h"
#include "game-analysis.h"


void initStartPos(void);
void parsePos(char* line);
void parseGo(char* line);
void uciOk(void);
void uciLoop(void);
void readInput(void);

#endif