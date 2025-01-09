#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdio.h>
#include <time.h>

// WEB gets no libraries
#ifndef WEB

// _WIN32 needs different libraries than Unix
#ifdef _WIN32
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