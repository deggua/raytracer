#include <time.h>
#include <unistd.h>

void SleepMS(u64 miliseconds)
{
    struct timespec ts = {
        .tv_sec  = (i64)miliseconds / 1000,
        .tv_nsec = ((i64)miliseconds % 1000) * 1000 * 1000,
    };

    nanosleep(&ts, NULL);
}
