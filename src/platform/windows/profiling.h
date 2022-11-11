#include <windows.h>

#include "common/common.h"

#define TIMEIT(desc, statement)                                                  \
    do {                                                                         \
        printf("Starting: " desc "\n");                                          \
        LARGE_INTEGER frequency;                                                 \
        LARGE_INTEGER start;                                                     \
        LARGE_INTEGER end;                                                       \
        double        interval;                                                  \
                                                                                 \
        QueryPerformanceFrequency(&frequency);                                   \
        QueryPerformanceCounter(&start);                                         \
                                                                                 \
        statement;                                                               \
                                                                                 \
        QueryPerformanceCounter(&end);                                           \
        interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart; \
                                                                                 \
        printf(desc " took %.2f seconds\n\n", interval);                         \
    } while (0)

#define TIMEBLOCK(desc)                        \
    do {                                       \
        printf("Starting: " desc "\n");        \
        LARGE_INTEGER frequency;               \
        LARGE_INTEGER start;                   \
        LARGE_INTEGER end;                     \
        double        interval;                \
                                               \
        QueryPerformanceFrequency(&frequency); \
        QueryPerformanceCounter(&start);       \
        clock_gettime(CLOCK_MONOTONIC, &specStart);

#define TIMEBLOCKEND(desc)                                                   \
                                                                             \
    QueryPerformanceCounter(&end);                                           \
    interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart; \
                                                                             \
    printf(desc " took %.2f seconds\n\n", interval);                         \
    }                                                                        \
    while (0)
