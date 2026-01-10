#define _POSIX_SOURCE
#include "platform.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include "string.h"

int getCurrentTime(void)
{
    struct timeval time_value;
    gettimeofday(&time_value, NULL);
    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

// Source: https://www.youtube.com/watch?v=gVGadWuBqEA
// Bluefever Software chess engine Vice
int isLineWaiting(void)
{
    if (!isatty(STDIN_FILENO))
    {
        return 0;
    }

    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
}

int readLine(char* buffer, int bufferSize, FILE* input)
{
    return fgets(buffer, bufferSize, input) == NULL;
}
