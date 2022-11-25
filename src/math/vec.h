#pragma once

#include <stdbool.h>

typedef enum {
    AXIS_X = 0,
    AXIS_U = AXIS_X,
    AXIS_R = AXIS_X,

    AXIS_Y     = 1,
    AXIS_V     = AXIS_Y,
    AXIS_THETA = AXIS_Y,

    AXIS_Z   = 2,
    AXIS_PHI = AXIS_Z,

    AXIS_W = 3,
    AXIS_LAST
} Axis;

typedef union {
    struct {
        f32 x, y;
    };

    struct {
        f32 u, v;
    };

    struct {
        f32 r, theta;
    };

    struct {
        f32 width, height;
    };

    f32 elem[2];
} vec2;

typedef union {
    struct {
        f32 x, y, z;
    };

    struct {
        f32 u, v, w;
    };

    struct {
        f32 r, g, b;
    };

    struct {
        f32 rho, theta, phi;
    };

    struct {
        vec2 xy;
        u32 : 32;
    };

    struct {
        u32 : 32;
        vec2 yz;
    };

    struct {
        vec2 uv;
        u32 : 32;
    };

    struct {
        u32 : 32;
        vec2 vw;
    };

    f32 elem[3];
} vec3;

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    f32 elems[4];
} vec4;

typedef vec2 point2;
typedef vec3 point3;
typedef vec4 point4;

typedef struct {
    vec2 x, y;
} basis2;

typedef struct {
    vec3 x, y, z;
} basis3;

typedef struct {
    vec4 x, y, z, w;
} basis4;

/* --- vec2 --- */

vec2 vec2_Set(f32 t);

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
vec2 vec2_Lerp(vec2 v1, vec2 v2, f32 t);

bool vec2_CompareMagnitudeEqual(vec2 v1, f32 mag);
bool vec2_CompareMagnitudeEqualR(f32 mag, vec2 v1);
bool vec2_CompareMagnitudeGreaterThan(vec2 v1, f32 mag);
bool vec2_CompareMagnitudeGreaterThanR(f32 mag, vec2 v1);
bool vec2_AlmostTheSame(vec2 v1, vec2 v2);

vec2 vec2_CartesianToPolar(vec2 cartesian);
vec2 vec2_PolarToCartesian(vec2 polar);

vec2 vec2_Make(f32 x, f32 y);

/* --- vec3 --- */

vec3 vec3_Set(f32 t);

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
vec3 vec3_Lerp(vec3 v1, vec3 v2, f32 t);

bool vec3_CompareMagnitudeEqual(vec3 v1, f32 mag);
bool vec3_CompareMagnitudeEqualR(f32 mag, vec3 v1);
bool vec3_CompareMagnitudeGreaterThan(vec3 v1, f32 mag);
bool vec3_CompareMagnitudeGreaterThanR(f32 mag, vec3 v1);
bool vec3_AlmostTheSame(vec3 v1, vec3 v2);

vec3 vec3_Reflect(vec3 vec, vec3 normal);
vec3 vec3_Refract(vec3 vec, vec3 normal, f32 refractRatio);

vec3 vec3_CartesianToSpherical(vec3 cartesian);
vec3 vec3_SphericalToCartesian(vec3 spherical);

vec3   vec3_Reorient(vec3 n, basis3 basis);
basis3 vec3_OrthonormalBasis(vec3 bx);
basis3 vec3_OrthonormalBasis_Inverse(basis3 orthonormal_basis);

vec3 vec3_Make(f32 x, f32 y, f32 z);

/* --- vec4 --- */

vec4 vec4_Set(f32 t);

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
vec4 vec4_Lerp(vec4 v1, vec4 v2, f32 t);

bool vec4_CompareMagnitudeEqual(vec4 v1, f32 mag);
bool vec4_CompareMagnitudeEqualR(f32 mag, vec4 v1);
bool vec4_CompareMagnitudeGreaterThan(vec4 v1, f32 mag);
bool vec4_CompareMagnitudeGreaterThanR(f32 mag, vec4 v1);
bool vec4_AlmostTheSame(vec4 v1, vec4 v2);

vec4 vec4_Make(f32 x, f32 y, f32 z, f32 w);

/* --- scalar --- */

f32  scalar_Multiply(f32 x, f32 y);
bool scalar_AlmostTheSame(f32 x, f32 y);
f32  scalar_Lerp(f32 a, f32 b, f32 t);

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

#define MAP _Generic
#define BIND(type, func) \
type:                    \
    func

/* clang-format off */

#define vadd(x, y) \
    MAP((x), \
        BIND(vec2, vec2_Add), \
        BIND(vec3, vec3_Add), \
        BIND(vec4, vec4_Add) \
       )((x), (y))

#define vsub(x, y) \
    MAP((x), \
        BIND(vec2, vec2_Subtract), \
        BIND(vec3, vec3_Subtract), \
        BIND(vec4, vec4_Subtract) \
       )((x), (y))

#define vmul(x, y) \
    MAP((x), \
        BIND(vec2, \
             MAP((y), \
                 BIND(vec2, vec2_MultiplyComponents), \
                 BIND(default, vec2_MultiplyScalar))), \
        BIND(vec3, \
             MAP((y), \
                 BIND(vec3, vec3_MultiplyComponents), \
                 BIND(default, vec3_MultiplyScalar))), \
        BIND(vec4, \
             MAP((y), \
                 BIND(vec4, vec4_MultiplyComponents), \
                 BIND(default, vec4_MultiplyScalar))), \
        BIND(default, \
             MAP((y), \
                 BIND(vec2, vec2_MultiplyScalarR), \
                 BIND(vec3, vec3_MultiplyScalarR), \
                 BIND(vec4, vec4_MultiplyScalarR), \
                 BIND(default, scalar_Multiply))) \
       )((x), (y))

#define vdiv(x, y) \
    MAP((x), \
        BIND(vec2, \
             MAP((y), \
                 BIND(vec2, vec2_DivideComponents), \
                 BIND(default, vec2_DivideScalar))), \
        BIND(vec3, \
             MAP((y), \
                 BIND(vec3, vec3_DivideComponents), \
                 BIND(default, vec3_DivideScalar))), \
        BIND(vec4, \
             MAP((y), \
                 BIND(vec4, vec4_DivideComponents), \
                 BIND(default, vec4_DivideScalar))) \
       )((x), (y))

#define vdot(x, y) \
    MAP((x), \
        BIND(vec2, vec2_DotProduct), \
        BIND(vec3, vec3_DotProduct), \
        BIND(vec4, vec4_DotProduct) \
       )((x), (y))

#define vcross(x, y) \
    MAP((x), \
        BIND(vec2, vec2_CrossProduct), \
        BIND(vec3, vec3_CrossProduct)\
       )((x), (y))

#define vmag(x) \
    MAP((x), \
        BIND(vec2, vec2_Magnitude), \
        BIND(vec3, vec3_Magnitude), \
        BIND(vec4, vec4_Magnitude) \
       )((x))

#define vmag2(x) \
    MAP((x), \
        BIND(vec2, vec2_MagnitudeSquared), \
        BIND(vec3, vec3_MagnitudeSquared), \
        BIND(vec4, vec4_MagnitudeSquared) \
       )((x))

#define vnorm(x) \
    MAP((x), \
        BIND(vec2, vec2_Normalize), \
        BIND(vec3, vec3_Normalize), \
        BIND(vec4, vec4_Normalize) \
       )((x))

#define vlerp(a, b, t) \
    MAP((a), \
        BIND(vec2, vec2_Lerp), \
        BIND(vec3, vec3_Lerp), \
        BIND(vec4, vec4_Lerp), \
        BIND(default, scalar_Lerp) \
    )((a), (b), (t))

// Test for equality
// vec, f32 := vec magnitude equality
// vec, vec := vec component equality
#define vequ(x, y) \
    MAP((x), \
        BIND(vec2, \
             MAP((y), \
                 BIND(vec2, vec2_AlmostTheSame), \
                 BIND(default, vec2_CompareMagnitudeEqual))), \
        BIND(vec3, \
             MAP((y), \
                 BIND(vec3, vec3_AlmostTheSame), \
                 BIND(default, vec3_CompareMagnitudeEqual))), \
        BIND(vec4, \
             MAP((y), \
                 BIND(vec4, vec4_AlmostTheSame), \
                 BIND(default, vec4_CompareMagnitudeEqual))), \
        BIND(default, \
             MAP((y), \
                 BIND(vec2, vec2_CompareMagnitudeEqualR), \
                 BIND(vec3, vec3_CompareMagnitudeEqualR), \
                 BIND(vec4, vec4_CompareMagnitudeEqualR), \
                 BIND(default, scalar_AlmostTheSame))) \
       )((x), (y))

#define vgt(x, y) \
    MAP((x), \
        BIND(vec2, vec2_CompareMagnitudeGreaterThan), \
        BIND(vec3, vec3_CompareMagnitudeGreaterThan), \
        BIND(vec4, vec4_CompareMagnitudeGreaterThan), \
        BIND(default, \
             MAP((y), \
                 BIND(vec2, vec2_CompareMagnitudeGreaterThanR), \
                 BIND(vec3, vec3_CompareMagnitudeGreaterThanR), \
                 BIND(vec4, vec4_CompareMagnitudeGreaterThanR))) \
       )((x), (y))

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

#define vec(...) GETOVERLOAD(__VA_ARGS__, _, _, _, vec4_Make, vec3_Make, vec2_Make)(__VA_ARGS__)
