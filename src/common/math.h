#pragma once

#include <float.h>
#include <math.h>
#include <stdbool.h>

#define PI32    ((f32)M_PI)
#define PI64    ((f64)M_PI)
#define EPSILON (FLT_EPSILON)

// TODO: make these compatible with different compilers and add some fallback for ISO C
#ifndef INF
#    define INF (__builtin_inff())
#endif

#ifndef NAN
#    define NAN (__builtin_nanf(""))
#endif

#define POWF_5(base) ((base) * (base) * (base) * (base) * (base))
#define POWF_4(base) ((base) * (base) * (base) * (base))
#define POWF_3(base) ((base) * (base) * (base))
#define POWF_2(base) ((base) * (base))

#define POWF_(base, exp) (POWF_##exp((base)))
#define POWF(base, exp)  (POWF_((base), exp))

f32  clampf(f32 x, f32 min, f32 max);
f32  radiansf(f32 degrees);
f32  degreesf(f32 radians);
f32  minf(f32 x, f32 y);
f32  maxf(f32 x, f32 y);
bool equalf(f32 a, f32 b);
