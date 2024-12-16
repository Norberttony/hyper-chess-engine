#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdio.h>
#include <time.h>

// WEB gets no libraries
#ifndef WEB

// by default, compiling it to windows. technically not 64 bit either, but... one day.
// there's probably a better way of doing this, but I haven't found it yet.
#define WIN64

// Win64 needs different libraries than Unix
#ifdef WIN64
#include "windows.h"
#else
#include "sys/time.h"
#include "sys/select.h"
#include "unistd.h"
#include "string.h"
#endif

#endif


int getCurrentTime(void);
int inputIsWaiting(void);

#endif