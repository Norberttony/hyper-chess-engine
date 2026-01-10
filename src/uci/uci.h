#ifndef UCI_HEADER
#define UCI_HEADER

// starts waiting for user input
void uciLoop(void);

// determines whether or not user has indicated to stop thinking
void pollStop(void);

// reads the string as a consecutive sequence of long algebraic notation
// plays each of the given moves on the board
void loadLAN(char* str);

#endif
