#ifndef UCI_HEADER
#define UCI_HEADER

// no idea if this is right, but unistd.h defines STDIN_FILENO for POSIX systems, and since
// windows does not support POSIX natively, it receives an explicit macro for defining STDIN_FILENO
#ifndef _WIN32
#include <unistd.h>
#else
#define STDIN_FILENO 0
#endif

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