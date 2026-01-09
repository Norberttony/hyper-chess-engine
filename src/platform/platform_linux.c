#include "platform.h"
#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include "string.h"

// returns the current time in ms
int getCurrentTime(void)
{
    struct timeval time_value;
    gettimeofday(&time_value, NULL);
    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

// Source: https://www.youtube.com/watch?v=gVGadWuBqEA
// Bluefever Software chess engine Vice
// Modified the windows version to only listen to keyboard down events.
int inputIsWaiting(void)
{
    // ignore the waiting input and finish the current operation if there is no
    // terminal actively open. This is a shoddy fix for getting the makefile to
    // work when piping commands.
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
