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
#ifdef WEB
#include <emscripten.h>
#endif

#include "think.h"
#include "defines.h"
#include "perft.h"
#include "test-suite.h"
#include "pins.h"


// starts waiting for user input
void uciLoop(void);

// reads the input buffer to see if the engine should pause what it is doing and consider user input
void readInput(void);

// reads the string as a consecutive sequence of long algebraic notation
// plays each of the given moves on the board
void loadLAN(char* str);

#endif
