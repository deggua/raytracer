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

// TODO: this doesn't appear to be faster than powf on -Ofast + unsafe math
// needs more testing, but this is interesting
#define POWF_2(base) ((base) * (base))
#define POWF_3(base) (POWF_2((base)) * (base))
#define POWF_4(base) (POWF_2((base)) * POWF_2((base)))
#define POWF_5(base) (POWF_4((base)) * (base))

#define POWF_(base, exp) (POWF_##exp((base)))
#if 0
#    define POWF(base, exp) (POWF_((base), exp))
#else
#    define POWF(base, exp) (powf((base), (exp)))
#endif

f32  clampf(f32 x, f32 min, f32 max);
f32  radiansf(f32 degrees);
f32  degreesf(f32 radians);
f32  minf(f32 x, f32 y);
f32  maxf(f32 x, f32 y);
bool equalf(f32 a, f32 b);
