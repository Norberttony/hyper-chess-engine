#include "utils.h"
#ifndef WEB
    // _WIN32 needs different libraries than Unix
    #ifdef _WIN32
        #include "windows.h"
    #else
        #define _POSIX_SOURCE
        #include "sys/time.h"
        #include "sys/select.h"
        #include "unistd.h"
        #include "string.h"
    #endif
#else
    // web libraries
    #include <emscripten.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <time.h>

// returns the current time in ms
int getCurrentTime(void)
{
#ifdef WEB
        return 1000 * clock() / CLOCKS_PER_SEC;
#else
    #ifdef _WIN32
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
// to-do: This has different behavior... for all three versions... which isn't
// a good thing. Windows version detects key presses (ConsoleInputs) and Linux
// version detects buffered inputs and Web modifies a specific JS object.
int inputIsWaiting(void)
{
#ifdef WEB
    // since this emscripten code is significantly slower than the windows/linux
    // build, there's an extra delay here.
    static int web_additional_delay = 0;
    if (++web_additional_delay < 100)
    {
        return 0;
    }
    web_additional_delay = 0;

    // allow browser to catch up
    emscripten_sleep(1);

    // check if browser added any new inputs
    int v = EM_ASM_INT({
        return channel.input != "" ? 1 : 0;
    });
    return v;
#else

#ifndef _WIN32
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

#else

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

#endif
#endif
}
