
#include "utils.h"

// returns the current time in ms
int getCurrentTime(void)
{
#ifdef WEB
        return 1000 * clock() / CLOCKS_PER_SEC;
#else
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
#endif
}


// Source: https://www.youtube.com/watch?v=gVGadWuBqEA
// Bluefever Software chess engine Vice
int inputIsWaiting(void)
{
    // temporary solution, currently WEB does not search infinitely
#ifdef WEB
    return 0;
#else

#ifndef WIN64
    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(filno(stdin), &readfds));

#else

    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init)
    {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe)
        {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe)
    {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
        {
            return 1;
        }
        return dw;
    }
    else
    {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
#endif
}
