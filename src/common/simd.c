#include "simd.h"

#include <immintrin.h>

#include "common/vec.h"

#if 0
// cross product:
// see https://geometrian.com/programming/tutorials/cross-product/index.php

// this can convert from vec3 -> simd -> vec3
// NOTE: using this + hand written intrinsics is slower than clang's optimized output of the simple cross product formula
// this seems to be because it's bottlenecked on the port than handles the convesion from f32[3] -> m128. Storing vec3
// internally as a vec4 has not been tried yet but may help/hurt the performance
// (requires more space in memory -> worse cache performance, requires larger alignment (16 vs 4) -> worse memory layout)
// see https://deplinenoise.files.wordpress.com/2015/03/gdc2015_afredriksson_simd.pdf although idk if it applies anymore

vec3 vec3_cross(vec3 v1, vec3 v2)
{
    const m128  v1_m128 = _mm_set_ps(0, v1.z, v1.y, v1.x);
    const m128  v2_m128 = _mm_set_ps(0, v2.z, v2.y, v2.x);
    const m128  cp_m128 = cross_product(v1_m128, v2_m128);
    const f32x4 cp      = {.m128 = cp_m128};

    return (vec3){
        .x = cp.f32[0],
        .y = cp.f32[1],
        .z = cp.f32[2],
    };
}
#endif
