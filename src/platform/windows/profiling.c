#include "platform/profiling.h"

#include <windows.h>

typedef struct Stopwatch {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER stop;
} Stopwatch;

Stopwatch* Stopwatch_New(void)
{
    return calloc(1, sizeof(Stopwatch));
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
    QueryPerformanceCounter(&stopwatch->end);
}

i64 Stopwatch_Elapsed(Stopwatch* stopwatch, Stopwatch_Timescale timescale)
{
    // TODO: fill this out
    // interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

#if 0
    switch (timescale) {
        case STOPWATCH_SECONDS: {
            return stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
        } break;

        case STOPWATCH_MILISECONDS: {
            i64 seconds_elapsed = stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
            i64 milis_elapsed   = (stopwatch->stop_time.tv_nsec - stopwatch->start_time.tv_nsec) * 1000 * 1000;
            return seconds_elapsed * 1000 + milis_elapsed;
        } break;

        case STOPWATCH_MICROSECONDS: {
            i64 seconds_elapsed = stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
            i64 micros_elapsed  = (stopwatch->stop_time.tv_nsec - stopwatch->start_time.tv_nsec) * 1000;
            return seconds_elapsed * 1000 * 1000 + micros_elapsed;
        } break;

        case STOPWATCH_NANOSECONDS: {
            i64 seconds_elapsed = stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
            i64 nanos_elapsed   = stopwatch->stop_time.tv_nsec - stopwatch->start_time.tv_nsec;
            return seconds_elapsed * 1000 * 1000 * 1000 + nanos_elapsed;
        } break;

        default:
            return -1;
    }
#endif

    return 0;
}
