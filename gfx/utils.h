#pragma once

#include <stdlib.h>

#define PI 3.141592653589793f

static inline float clampf(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static inline float randf() {
    return rand() / ((float)RAND_MAX + 1.0f);
}

static inline float randrf(float min, float max) {
    return min + (max - min) * randf();
}

static inline float radiansf(float degrees) {
    return degrees * PI / 180.0f;
}

static inline float degreesf(float radians) {
    return radians * 180.0f / PI;
}

#define min(x, y) (x > y ? y : x)
#define max(x, y) (x > y ? x : y)

#define smallestf(x, y) (fabsf(x) > fabsf(y) ? y : x)
#define largestf(x, y) (fabsf(x) > fabsf(y) ? x : y)

#define within(val, tmin, tmax) (val > tmin && val < tmax)
