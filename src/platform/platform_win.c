#include "platform.h"
#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

char inputBuffer[BUFFER_SIZE] = { 0 };
int returnFromBuffer = 0;
int inputBufferIdx = 0;

int getCurrentTime(void)
{
    return GetTickCount();
}

// Source: https://www.youtube.com/watch?v=gVGadWuBqEA
// Bluefever Software chess engine Vice
// Modified the windows version to only listen to keyboard down events.
int isLineWaiting(void)
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
                    char ch = records[i].Event.KeyEvent.uChar.AsciiChar;
                    if (ch != 0 && inputBufferIdx + 1 < BUFFER_SIZE)
                    {
                        inputBuffer[inputBufferIdx++] = ch;
                        inputBuffer[inputBufferIdx] = '\0';

                        // give user the impression that they are typing into
                        // the terminal even though their inputs get eaten
                        printf("%c", ch);
                    }

                    // handle backspace
                    if (records[i].Event.KeyEvent.wVirtualKeyCode == VK_BACK)
                    {
                        printf(" ");
                        printf("%c", ch);

                        // remove the \b character that was just added
                        if (inputBufferIdx > 1)
                        {
                            inputBufferIdx--;
                            inputBuffer[inputBufferIdx--] = '\0';
                        }
                    }

                    if (ch == '\r' || ch == '\n')
                    {
                        ReadConsoleInput(inh, &records[0], 1, &amt);
                        returnFromBuffer = 1;
                        printf("\n");
                        return 1;
                    }
                }
                // eat input
                ReadConsoleInput(inh, &records[0], 1, &amt);
            }
        }
        return 0;
    }
}

int readLine(char* buffer, int bufferSize, FILE* input)
{
    if (returnFromBuffer)
    {
        // must copy over from stored input buffer into the given buffer
        for (int i = 0; i == 0 || inputBuffer[i - 1] != '\0'; i++)
        {
            // avoid overflowing the given buffer
            if (i + 1 == bufferSize)
            {
                buffer[i] = '\0';
                break;
            }
            buffer[i] = inputBuffer[i];
        }

        returnFromBuffer = 0;
        inputBufferIdx = 0;

        return 0;
    }
    else
    {
        return fgets(buffer, bufferSize, input) == NULL;
    }
}
