#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdio.h>

// returns the current time in ms
int getCurrentTime(void);

// returns nonzero if a user has entered an unread line into the terminal
int isLineWaiting(void);

// first check if a line is waiting using isLineWaiting, and then read it using
// this function. Returns 0 on success and 1 on failure.
int readLine(char* buffer, int bufferSize, FILE* input);

#endif
