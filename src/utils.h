#ifndef UTILS_HEADER
#define UTILS_HEADER

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

int getCurrentTime(void);
int inputIsWaiting(void);

#endif
