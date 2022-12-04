#pragma once

typedef struct Stopwatch Stopwatch;

typedef enum {
    STOPWATCH_SECONDS,
    STOPWATCH_MILISECONDS,
    STOPWATCH_MICROSECONDS,
    STOPWATCH_NANOSECONDS,
} Stopwatch_Timescale;

Stopwatch* Stopwatch_New(void);
void       Stopwatch_Delete(Stopwatch* stopwatch);
void       Stopwatch_Start(Stopwatch* stopwatch);
void       Stopwatch_Stop(Stopwatch* stopwatch);
i64        Stopwatch_Elapsed(Stopwatch* stopwatch, Stopwatch_Timescale timescale);

intern const char* Stopwatch_Units[] = {
    [STOPWATCH_SECONDS]      = "s",
    [STOPWATCH_MILISECONDS]  = "ms",
    [STOPWATCH_MICROSECONDS] = "us",
    [STOPWATCH_NANOSECONDS]  = "ns",
};

#define STOPWATCH_TIMESCALE_UNIT(ts) (Stopwatch_Units[ts])

#define TIMEIT(sw, ts, desc, ...)                                                                           \
    do {                                                                                                    \
        printf("Starting :: " desc "\n");                                                                   \
        Stopwatch_Start(sw);                                                                                \
        __VA_ARGS__;                                                                                        \
        Stopwatch_Stop(sw);                                                                                 \
        i64 elapsed_ = Stopwatch_Elapsed(sw, ts);                                                           \
        printf("Finished :: " desc " :: (" I64_DEC_FMT " %s)\n\n", elapsed_, STOPWATCH_TIMESCALE_UNIT(ts)); \
    } while (0)
