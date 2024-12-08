#ifndef CDATAPACK_INCLUDE_H
#define CDATAPACK_INCLUDE_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#define WINDOWS
#endif

#ifdef __APPLE__
#define APPLE
#endif

#if defined(__linux__) || defined(__unix__)
#define UNIX
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#ifdef APPLE
#error I don't have access to MacOS so it isn't supported yet.
#endif

#ifdef UNIX
#include <limits.h>
#include <unistd.h>
#endif

#endif