#include "platform.h"
#include <emscripten.h>
#include <stdio.h>
#include <time.h>

// returns the current time in ms
int getCurrentTime(void)
{
    return 1000 * clock() / CLOCKS_PER_SEC;
}

int inputIsWaiting(void)
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
