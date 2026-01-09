#include "platform.h"
#include <windows.h>
#include <stdio.h>

int getCurrentTime(void)
{
    return GetTickCount();
}

// Source: https://www.youtube.com/watch?v=gVGadWuBqEA
// Bluefever Software chess engine Vice
// Modified the windows version to only listen to keyboard down events.
int inputIsWaiting(void)
{
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw = 0;

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
        // A slight modification from the copied code: when PeekNamedPipe fails
        // then '1' is not returned. When makefile was executing the program to
        // make the profile build, it got hung up because, probably, it is
        // running the program without stdin (in a "detached" state)... so if
        // PeekNamedPipe fails, 0 is returned (input is not waiting). I don't
        // know the full effects of removing PeekNamedPipe, but functionality
        // looks the same.
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
        {
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
}
