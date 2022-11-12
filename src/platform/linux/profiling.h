#include <time.h>

#define TIMEIT(desc, statement)                                                \
    do {                                                                       \
        printf("Starting: " desc "\n");                                        \
        struct timespec specStart;                                             \
        struct timespec specEnd;                                               \
        clock_gettime(CLOCK_MONOTONIC, &specStart);                            \
                                                                               \
        statement;                                                             \
                                                                               \
        clock_gettime(CLOCK_MONOTONIC, &specEnd);                              \
        f32 timeDeltaSec  = (f32)(specEnd.tv_sec - specStart.tv_sec);          \
        f32 timeDeltaFrac = (f32)(specEnd.tv_nsec - specStart.tv_nsec) * 1e-9; \
        f32 timeDelta     = timeDeltaSec + timeDeltaFrac;                      \
                                                                               \
        printf(desc " took %.2f seconds\n\n", timeDelta);                      \
    } while (0)

#define TIMEBLOCK(desc)                 \
    do {                                \
        printf("Starting: " desc "\n"); \
        struct timespec specStart;      \
        struct timespec specEnd;        \
        clock_gettime(CLOCK_MONOTONIC, &specStart);

#define TIMEBLOCKEND(desc)                                                 \
                                                                           \
    clock_gettime(CLOCK_MONOTONIC, &specEnd);                              \
    f32 timeDeltaSec  = (f32)(specEnd.tv_sec - specStart.tv_sec);          \
    f32 timeDeltaFrac = (f32)(specEnd.tv_nsec - specStart.tv_nsec) * 1e-9; \
    f32 timeDelta     = timeDeltaSec + timeDeltaFrac;                      \
                                                                           \
    printf(desc " took %.2f seconds\n\n", timeDelta);                      \
    }                                                                      \
    while (0)
