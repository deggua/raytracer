#include <time.h>
#include <unistd.h>

void SleepMS(u64 miliseconds)
{
    struct timespec ts = {
        .tv_sec  = miliseconds / 1000,
        .tv_nsec = (miliseconds % 1000) * 1000 * 1000,
    };

    nanosleep(&ts, NULL);
}
