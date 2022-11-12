#include "math.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "common/common.h"

f32 clampf(f32 x, f32 min, f32 max)
{
    if (x < min) {
        return min;
    } else if (x > max) {
        return max;
    } else {
        return x;
    }
}

f32 radiansf(f32 degrees)
{
    return degrees * PI / 180.0f;
}

f32 degreesf(f32 radians)
{
    return radians * 180.0f / PI;
}

f32 minf(f32 x, f32 y)
{
    if (x > y) {
        return y;
    } else {
        return x;
    }
}

f32 maxf(f32 x, f32 y)
{
    if (x > y) {
        return x;
    } else {
        return y;
    }
}

bool equalf(f32 a, f32 b)
{
    const f32 absA = fabsf(a);
    const f32 absB = fabsf(b);
    const f32 diff = fabsf(a - b);

    if (a == b) { // shortcut, handles infinities
        return true;
    } else if (a == 0 || b == 0 || (absA + absB < FLT_MIN)) {
        // a or b is zero or both are extremely close to it
        // relative error is less meaningful here
        return diff < (EPSILON * FLT_MIN);
    } else { // use relative error
        return diff / minf((absA + absB), FLT_MAX) < EPSILON;
    }
}
