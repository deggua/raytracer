#pragma once

#include <stdbool.h>

#include "common/common.h"
#include "common/simd.h"

typedef enum
{
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2,
    AXIS_W = 3,
    AXIS_LAST
} Axis;

typedef enum
{
    VEC_EQU,
    VEC_GT,
} VecCompare;

typedef union {
    struct {
        f32 x;
        f32 y;
    };
    f32 v[2];
} vec2;

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 v[3];
} vec3;

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    f32 v[4];
} vec4;

// TODO: enable this? useful for conversion between vectors
#if 0
typedef union {
    m128 m128;
    vec2 vec2;
    vec3 vec3;
    vec4 vec4;
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    f32 v[4];
} vecu;
#endif

typedef vec2 point2;
typedef vec3 point3;
typedef vec4 point4;

/* --- vec2 --- */
vec2 vec2_Add(vec2 v1, vec2 v2);
vec2 vec2_Subtract(vec2 v1, vec2 v2);

vec2 vec2_MultiplyComponents(vec2 v1, vec2 v2);
vec2 vec2_MultiplyScalar(vec2 vec, f32 scalar);
vec2 vec2_MultiplyScalarR(f32 scalar, vec2 vec);

vec2 vec2_DivideComponents(vec2 vdividend, vec2 vdivisor);
vec2 vec2_DivideScalar(vec2 vec, f32 scalar);

f32  vec2_DotProduct(vec2 v1, vec2 v2);
vec3 vec2_CrossProduct(vec2 v1, vec2 v2);

f32 vec2_Magnitude(vec2 vec);
f32 vec2_MagnitudeSquared(vec2 vec);

vec2 vec2_Normalize(vec2 vec);

bool vec2_CompareMagnitudeEqual(vec2 v1, f32 mag);
bool vec2_CompareMagnitudeEqualR(f32 mag, vec2 v1);
bool vec2_CompareMagnitudeGreaterThan(vec2 v1, f32 mag);
bool vec2_CompareMagnitudeGreaterThanR(f32 mag, vec2 v1);
bool vec2_AlmostTheSame(vec2 v1, vec2 v2);

/* --- vec3 --- */

vec3 vec3_Add(vec3 v1, vec3 v2);
vec3 vec3_Subtract(vec3 v1, vec3 v2);

vec3 vec3_MultiplyComponents(vec3 v1, vec3 v2);
vec3 vec3_MultiplyScalar(vec3 vec, f32 scalar);
vec3 vec3_MultiplyScalarR(f32 scalar, vec3 vec);

vec3 vec3_DivideComponents(vec3 vdividend, vec3 vdivisor);
vec3 vec3_DivideScalar(vec3 vec, f32 scalar);

f32  vec3_DotProduct(vec3 v1, vec3 v2);
vec3 vec3_CrossProduct(vec3 v1, vec3 v2);

f32 vec3_Magnitude(vec3 vec);
f32 vec3_MagnitudeSquared(vec3 vec);

vec3 vec3_Normalize(vec3 vec);

bool vec3_CompareMagnitudeEqual(vec3 v1, f32 mag);
bool vec3_CompareMagnitudeEqualR(f32 mag, vec3 v1);
bool vec3_CompareMagnitudeGreaterThan(vec3 v1, f32 mag);
bool vec3_CompareMagnitudeGreaterThanR(f32 mag, vec3 v1);
bool vec3_AlmostTheSame(vec3 v1, vec3 v2);

vec3 vec3_Random(f32 min, f32 max);
vec3 vec3_RandomInUnitDisc(void); // TODO: should I make it explicit that this is a disc in the XY-plane?
vec3 vec3_RandomInUnitSphere(void);
vec3 vec3_RandomOnUnitSphere(void);

vec3 vec3_Reflect(vec3 vec, vec3 normal);
vec3 vec3_Refract(vec3 vec, vec3 normal, f32 refractRatio);

/* --- vec4 --- */

vec4 vec4_Add(vec4 v1, vec4 v2);
vec4 vec4_Subtract(vec4 v1, vec4 v2);

vec4 vec4_MultiplyComponents(vec4 v1, vec4 v2);
vec4 vec4_MultiplyScalar(vec4 vec, f32 scalar);
vec4 vec4_MultiplyScalarR(f32 scalar, vec4 vec);

vec4 vec4_DivideComponents(vec4 vdividend, vec4 vdivisor);
vec4 vec4_DivideScalar(vec4 vec, f32 scalar);

f32  vec4_DotProduct(vec4 v1, vec4 v2);
vec4 vec4_CrossProduct(vec4 v1, vec4 v2);

f32 vec4_Magnitude(vec4 vec);
f32 vec4_MagnitudeSquared(vec4 vec);

vec4 vec4_Normalize(vec4 vec);

bool vec4_CompareMagnitudeEqual(vec4 v1, f32 mag);
bool vec4_CompareMagnitudeEqualR(f32 mag, vec4 v1);
bool vec4_CompareMagnitudeGreaterThan(vec4 v1, f32 mag);
bool vec4_CompareMagnitudeGreaterThanR(f32 mag, vec4 v1);
bool vec4_AlmostTheSame(vec4 v1, vec4 v2);

#define VEC2_FMT      "<%.02f, %.02f>"
#define VEC2_ARG(vec) vec.x, vec.y

#define VEC3_FMT      "<%.02f, %.02f, %.02f>"
#define VEC3_ARG(vec) vec.x, vec.y, vec.z

#define VEC4_FMT      "<%.02f, %.02f, %.02f, %.02f>"
#define VEC4_ARG(vec) vec.x, vec.y, vec.z, vec.w

#define AXIS_FMT "%s"
#define AXIS_ARG(axis)       \
    ((const char*[]){        \
        [AXIS_X] = "x-axis", \
        [AXIS_Y] = "y-axis", \
        [AXIS_Z] = "z-axis", \
        [AXIS_W] = "w-axis", \
    })[axis]

#define OVERLOAD_SELECT _Generic
#define OVERLOAD_MAP(type, func) \
type:                            \
    func

/* clang-format off */
#define vadd(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_Add), \
        OVERLOAD_MAP(vec3, vec3_Add), \
        OVERLOAD_MAP(vec4, vec4_Add) \
    )(x, y)

#define vsub(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_Subtract), \
        OVERLOAD_MAP(vec3, vec3_Subtract), \
        OVERLOAD_MAP(vec4, vec4_Subtract) \
    )(x, y)

#define vmul(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_MultiplyComponents), \
                OVERLOAD_MAP(default, vec2_MultiplyScalar))), \
        OVERLOAD_MAP(vec3, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec3, vec3_MultiplyComponents), \
                OVERLOAD_MAP(default, vec3_MultiplyScalar))), \
        OVERLOAD_MAP(vec4, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec4, vec4_MultiplyComponents), \
                OVERLOAD_MAP(default, vec4_MultiplyScalar))), \
        OVERLOAD_MAP(default, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_MultiplyScalarR), \
                OVERLOAD_MAP(vec3, vec3_MultiplyScalarR), \
                OVERLOAD_MAP(vec4, vec4_MultiplyScalarR), \
                OVERLOAD_MAP(default, 0))) \
    )(x, y)

#define vdiv(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_DivideComponents), \
                OVERLOAD_MAP(default, vec2_DivideScalar))), \
        OVERLOAD_MAP(vec3, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec3, vec3_DivideComponents), \
                OVERLOAD_MAP(default, vec3_DivideScalar))), \
        OVERLOAD_MAP(vec4, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec4, vec4_DivideComponents), \
                OVERLOAD_MAP(default, vec4_DivideScalar))) \
    )(x, y)

#define vdot(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_DotProduct), \
        OVERLOAD_MAP(vec3, vec3_DotProduct), \
        OVERLOAD_MAP(vec4, vec4_DotProduct) \
    )(x, y)

#define vcross(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_CrossProduct), \
        OVERLOAD_MAP(vec3, vec3_CrossProduct)\
    )(x, y)

#define vmag(x) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_Magnitude), \
        OVERLOAD_MAP(vec3, vec3_Magnitude), \
        OVERLOAD_MAP(vec4, vec4_Magnitude) \
    )(x, y)

#define vmag2(x) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_MagnitudeSquared), \
        OVERLOAD_MAP(vec3, vec3_MagnitudeSquared), \
        OVERLOAD_MAP(vec4, vec4_MagnitudeSquared) \
    )(x, y)

#define vnorm(x) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_Normalize), \
        OVERLOAD_MAP(vec3, vec3_Normalize), \
        OVERLOAD_MAP(vec4, vec4_Normalize) \
    )(x)

#define vequ(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_AlmostTheSame), \
                OVERLOAD_MAP(default, vec2_CompareMagnitudeEqual))), \
        OVERLOAD_MAP(vec3, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec3, vec3_AlmostTheSame), \
                OVERLOAD_MAP(default, vec3_CompareMagnitudeEqual))), \
        OVERLOAD_MAP(vec4, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec4, vec4_AlmostTheSame), \
                OVERLOAD_MAP(default, vec4_CompareMagnitudeEqual))), \
        OVERLOAD_MAP(default, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_CompareMagnitudeEqualR), \
                OVERLOAD_MAP(vec3, vec3_CompareMagnitudeEqualR), \
                OVERLOAD_MAP(vec4, vec4_CompareMagnitudeEqualR), \
                OVERLOAD_MAP(default, 0))) \
    )(x, y)

#define vgt(x, y) \
    OVERLOAD_SELECT((x), \
        OVERLOAD_MAP(vec2, vec2_CompareMagnitudeGreaterThan), \
        OVERLOAD_MAP(vec3, vec3_CompareMagnitudeGreaterThan), \
        OVERLOAD_MAP(vec4, vec4_CompareMagnitudeGreaterThan), \
        OVERLOAD_MAP(default, \
            OVERLOAD_SELECT((y), \
                OVERLOAD_MAP(vec2, vec2_CompareMagnitudeGreaterThanR), \
                OVERLOAD_MAP(vec3, vec3_CompareMagnitudeGreaterThanR), \
                OVERLOAD_MAP(vec4, vec4_CompareMagnitudeGreaterThanR))) \
    )(x, y)

#define vneq(x, y)  (!vequ((x), (y)))
#define vlteq(x, y) (!vgt((x), (y)))
#define vlt(x, y)   (!vgt((x), (y)) && !vequ((x), (y)))
#define vgteq(x, y) (vgt((x), (y)) || vequ((x), (y)))

/* clang-format on */

#define GETOVERLOAD(IGNORE1, IGNORE2, IGNORE3, INGORE4, IGNORE5, IGNORE6, IGNORE7, NAME, ...) NAME

#define VSUM_2(v1, v2)                         vadd((v1), (v2))
#define VSUM_3(v1, v2, v3)                     VSUM_2(VSUM_2((v1), (v2)), (v3))
#define VSUM_4(v1, v2, v3, v4)                 VSUM_2(VSUM_2((v1), (v2)), VSUM_2((v3), (v4)))
#define VSUM_5(v1, v2, v3, v4, v5)             VSUM_2(VSUM_4((v1), (v2), (v3), (v4)), (v5))
#define VSUM_6(v1, v2, v3, v4, v5, v6)         VSUM_2(VSUM_4((v1), (v2), (v3), (v4)), VSUM_2((v5), (v6)))
#define VSUM_7(v1, v2, v3, v4, v5, v6, v7)     VSUM_2(VSUM_4((v1), (v2), (v3), (v4)), VSUM_2((v5), (v6)), (v7))
#define VSUM_8(v1, v2, v3, v4, v5, v6, v7, v8) VSUM_2(VSUM_4((v1), (v2), (v3), (v4)), VSUM_4((v5), (v6), (v7), (v8)))

#define vsum(v1, ...) \
    GETOVERLOAD(__VA_ARGS__, VSUM_8, VSUM_7, VSUM_6, VSUM_5, VSUM_4, VSUM_3, VSUM_2, IGNORE)((v1), __VA_ARGS__)

#define VPROD_2(v1, v2)                     vmul((v1), (v2))
#define VPROD_3(v1, v2, v3)                 VPROD_2(VPROD_2((v1), (v2)), (v3))
#define VPROD_4(v1, v2, v3, v4)             VPROD_2(VPROD_2((v1), (v2)), VPROD_2((v3), (v4)))
#define VPROD_5(v1, v2, v3, v4, v5)         VPROD_2(VPROD_4((v1), (v2), (v3), (v4)), (v5))
#define VPROD_6(v1, v2, v3, v4, v5, v6)     VPROD_2(VPROD_4((v1), (v2), (v3), (v4)), VPROD_2((v5), (v6)))
#define VPROD_7(v1, v2, v3, v4, v5, v6, v7) VPROD_2(VPROD_4((v1), (v2), (v3), (v4)), VPROD_2((v5), (v6)), (v7))
#define VPROD_8(v1, v2, v3, v4, v5, v6, v7, v8) \
    VPROD_2(VPROD_4((v1), (v2), (v3), (v4)), VPROD_4((v5), (v6), (v7), (v8)))

#define vprod(v1, ...) \
    GETOVERLOAD(__VA_ARGS__, VPROD_8, VPROD_7, VPROD_6, VPROD_5, VPROD_4, VPROD_3, VPROD_2, IGNORE)((v1), __VA_ARGS__)
