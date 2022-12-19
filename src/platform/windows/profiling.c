#include "platform/profiling.h"

#include <windows.h>

typedef struct Stopwatch {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER stop;
} Stopwatch;

Stopwatch* Stopwatch_New(void)
{
    return (Stopwatch*)calloc(1, sizeof(Stopwatch));
}

void Stopwatch_Delete(Stopwatch* stopwatch)
{
    free(stopwatch);
}

void Stopwatch_Start(Stopwatch* stopwatch)
{
    QueryPerformanceFrequency(&stopwatch->frequency);
    QueryPerformanceCounter(&stopwatch->start);
}

void Stopwatch_Stop(Stopwatch* stopwatch)
{
    QueryPerformanceCounter(&stopwatch->stop);
}

i64 Stopwatch_Elapsed(Stopwatch* stopwatch, Stopwatch_Timescale timescale)
{
    switch (timescale) {
        case STOPWATCH_SECONDS: {
            i64 ticks_per_second = stopwatch->frequency.QuadPart;
            return (stopwatch->stop.QuadPart - stopwatch->start.QuadPart) / ticks_per_second;
        } break;

        case STOPWATCH_MILISECONDS: {
            i64 ticks_per_milisecond = stopwatch->frequency.QuadPart / 1000;
            return (stopwatch->stop.QuadPart - stopwatch->start.QuadPart) / ticks_per_milisecond;
        } break;

        case STOPWATCH_MICROSECONDS: {
            i64 ticks_per_microsecond = stopwatch->frequency.QuadPart / (1000 * 1000);
            return (stopwatch->stop.QuadPart - stopwatch->start.QuadPart) / ticks_per_microsecond;
        } break;

        case STOPWATCH_NANOSECONDS: {
            i64 ticks_per_nanosecond = stopwatch->frequency.QuadPart / (1000 * 1000 * 1000);
            return (stopwatch->stop.QuadPart - stopwatch->start.QuadPart) / ticks_per_nanosecond;
        } break;

        default:
            return -1;
    }

    return 0;
}
