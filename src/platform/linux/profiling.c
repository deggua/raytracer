#include "platform/profiling.h"

#include <time.h>

typedef struct Stopwatch {
    struct timespec start_time;
    struct timespec stop_time;
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
    clock_gettime(CLOCK_MONOTONIC, &stopwatch->start_time);
}

void Stopwatch_Stop(Stopwatch* stopwatch)
{
    clock_gettime(CLOCK_MONOTONIC, &stopwatch->stop_time);
}

i64 Stopwatch_Elapsed(Stopwatch* stopwatch, Stopwatch_Timescale timescale)
{
    // TODO: signed overflow handling

    switch (timescale) {
        case STOPWATCH_SECONDS: {
            return stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
        } break;

        case STOPWATCH_MILISECONDS: {
            i64 seconds_elapsed = stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
            i64 milis_elapsed   = (stopwatch->stop_time.tv_nsec - stopwatch->start_time.tv_nsec) / (1000 * 1000);
            return seconds_elapsed * 1000 + milis_elapsed;
        } break;

        case STOPWATCH_MICROSECONDS: {
            i64 seconds_elapsed = stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec;
            i64 micros_elapsed  = (stopwatch->stop_time.tv_nsec - stopwatch->start_time.tv_nsec) / 1000;
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
}
