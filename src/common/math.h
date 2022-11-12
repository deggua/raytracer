#pragma once

#include <stdbool.h>

#define PI      3.141592653589793f
#define EPSILON 1e-8

// TODO: make these compatible with different compilers and add some fallback for ISO C
#ifndef INF
#    define INF (__builtin_inff())
#endif

#ifndef NAN
#    define NAN (__builtin_nanf(""))
#endif

f32  clampf(f32 x, f32 min, f32 max);
f32  radiansf(f32 degrees);
f32  degreesf(f32 radians);
f32  minf(f32 x, f32 y);
f32  maxf(f32 x, f32 y);
bool equalf(f32 a, f32 b);
