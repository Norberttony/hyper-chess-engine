#ifndef UCI_HEADER
#define UCI_HEADER

// starts waiting for user input
void uciLoop(void);

// reads the input buffer to see if the engine should pause what it is doing and consider user input
void readInput(void);

// reads the string as a consecutive sequence of long algebraic notation
// plays each of the given moves on the board
void loadLAN(char* str);

#endif
