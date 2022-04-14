#pragma once

#include <stdint.h>
#include <stdlib.h>

#define PI 3.141592653589793f

static inline float clampf(float x, float min, float max)
{
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

static inline float radiansf(float degrees)
{
    return degrees * PI / 180.0f;
}

static inline float degreesf(float radians)
{
    return radians * 180.0f / PI;
}

static inline float minf(float x, float y)
{
    if (x > y) {
        return y;
    } else {
        return x;
    }
}

static inline float maxf(float x, float y)
{
    if (x > y) {
        return x;
    } else {
        return y;
    }
}
