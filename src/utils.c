
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
// Modified the windows version to only listen to keyboard down events.
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
        
        // determine if a key was pressed
        if (dw > 0)
        {
            INPUT_RECORD records[1];
            DWORD amt;
            PeekConsoleInput(inh, &records[0], 1, &amt);

            for (DWORD i = 0; i < amt; i++)
            {
                if (records[i].EventType == KEY_EVENT && records[i].Event.KeyEvent.bKeyDown)
                {
                    return 1;
                }
                else
                {
                    // eat input
                    ReadConsoleInput(inh, &records[0], 1, &amt);
                }
            }
        }
        return 0;
    }

#endif
#endif
}
