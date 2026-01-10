#include "platform.h"
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// returns the current time in ms
int getCurrentTime(void)
{
    return 1000 * clock() / CLOCKS_PER_SEC;
}

int isLineWaiting(void)
{
    // since this emscripten code is significantly slower there's an extra delay
    // here.
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
}

int readLine(char* buffer, int bufferSize, FILE* input)
{
    // ignores input file
    (void)input;

    // allow the browser to catch up
    emscripten_sleep(100);

    char* toCopy = (char*)EM_ASM_PTR({
        return channel.C_readline();
    });

    for (int i = 0; i == 0 || toCopy[i - 1] != '\0'; i++)
    {
        if (i == bufferSize)
        {
            buffer[i - 1] = '\0';
            break;
        }
        buffer[i] = toCopy[i];
    }
    free(toCopy);

    return 0;
}
