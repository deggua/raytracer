#include "vec.h"

#include <math.h>
#include <stdbool.h>

#include "common/math.h"
#include "common/random.h"

/* --- Vec2 Functions --- */

vec2 vec2_Add(vec2 v1, vec2 v2)
{
    return (vec2){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

vec2 vec2_Subtract(vec2 v1, vec2 v2)
{
    return (vec2){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

vec2 vec2_MultiplyScalar(vec2 vec, f32 scalar)
{
    return (vec2){
        .x = scalar * vec.x,
        .y = scalar * vec.y,
    };
}

vec2 vec2_MultiplyScalarR(f32 scalar, vec2 vec)
{
    return vec2_MultiplyScalar(vec, scalar);
}

vec2 vec2_MultiplyComponents(vec2 v1, vec2 v2)
{
    return (vec2){
        .x = v1.x * v2.x,
        .y = v1.y * v2.y,
    };
}

vec2 vec2_DivideScalar(vec2 vec, f32 scalar)
{
    return vec2_MultiplyScalar(vec, 1.0f / scalar);
}

vec2 vec2_DivideComponents(vec2 vdividend, vec2 vdivisor)
{
    return (vec2){
        .x = vdividend.x / vdivisor.x,
        .y = vdividend.y / vdivisor.y,
    };
}

f32 vec2_DotProduct(vec2 v1, vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

vec3 vec2_CrossProduct(vec2 v1, vec2 v2)
{
    return (vec3){
        .z = (v1.x * v2.y - v1.y * v2.x),
    };
}

f32 vec2_MagnitudeSquared(vec2 vec)
{
    return vec2_DotProduct(vec, vec);
}

f32 vec2_Magnitude(vec2 vec)
{
    return sqrtf(vec2_MagnitudeSquared(vec));
}

vec2 vec2_Normalize(vec2 vec)
{
    return vec2_DivideScalar(vec, vec2_Magnitude(vec));
}

bool vec2_CompareMagnitudeEqual(vec2 v1, f32 mag)
{
    const f32 v1mag = vec2_MagnitudeSquared(v1);
    return equalf(v1mag, mag * mag);
}

bool vec2_CompareMagnitudeEqualR(f32 mag, vec2 v1)
{
    return vec2_CompareMagnitudeEqual(v1, mag);
}

bool vec2_CompareMagnitudeGreaterThan(vec2 v1, f32 mag)
{
    const f32 v1mag = vec2_MagnitudeSquared(v1);
    return v1mag > mag;
}

bool vec2_CompareMagnitudeGreaterThanR(f32 mag, vec2 v1)
{
    return vec2_CompareMagnitudeGreaterThan(v1, mag);
}

bool vec2_AlmostTheSame(vec2 v1, vec2 v2)
{
    return equalf(v1.x, v2.x) && equalf(v1.y, v2.y);
}

/* --- Vec3 Functions --- */

vec3 vec3_Add(vec3 v1, vec3 v2)
{
    return (vec3){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
    };
}

vec3 vec3_Subtract(vec3 v1, vec3 v2)
{
    return (vec3){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
}

vec3 vec3_MultiplyScalar(vec3 vec, f32 scalar)
{
    return (vec3){
        .x = scalar * vec.x,
        .y = scalar * vec.y,
        .z = scalar * vec.z,
    };
}

vec3 vec3_MultiplyScalarR(f32 scalar, vec3 vec)
{
    return vec3_MultiplyScalar(vec, scalar);
}

vec3 vec3_MultiplyComponents(vec3 v1, vec3 v2)
{
    return (vec3){
        .x = v1.x * v2.x,
        .y = v1.y * v2.y,
        .z = v1.z * v2.z,
    };
}

vec3 vec3_DivideScalar(vec3 vec, f32 scalar)
{
    return vec3_MultiplyScalar(vec, 1.0f / scalar);
}

vec3 vec3_DivideComponents(vec3 vdividend, vec3 vdivisor)
{
    return (vec3){
        .x = vdividend.x / vdivisor.x,
        .y = vdividend.y / vdivisor.y,
        .z = vdividend.z / vdivisor.z,
    };
}

f32 vec3_DotProduct(vec3 v1, vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3 vec3_CrossProduct(vec3 v1, vec3 v2)
{
    return (vec3){
        .x = (v1.y * v2.z - v1.z * v2.y),
        .y = (v1.z * v2.x - v1.x * v2.z),
        .z = (v1.x * v2.y - v1.y * v2.x),
    };
}

f32 vec3_MagnitudeSquared(vec3 vec)
{
    return vec3_DotProduct(vec, vec);
}

f32 vec3_Magnitude(vec3 vec)
{
    return sqrtf(vec3_MagnitudeSquared(vec));
}

vec3 vec3_Normalize(vec3 vec)
{
    return vec3_DivideScalar(vec, vec3_Magnitude(vec));
}

bool vec3_CompareMagnitudeEqual(vec3 v1, f32 mag)
{
    const f32 v1mag = vec3_MagnitudeSquared(v1);
    return equalf(v1mag, mag * mag);
}

bool vec3_CompareMagnitudeEqualR(f32 mag, vec3 v1)
{
    return vec3_CompareMagnitudeEqual(v1, mag);
}

bool vec3_CompareMagnitudeGreaterThan(vec3 v1, f32 mag)
{
    const f32 v1mag = vec3_MagnitudeSquared(v1);
    return v1mag > mag;
}

bool vec3_CompareMagnitudeGreaterThanR(f32 mag, vec3 v1)
{
    return vec3_CompareMagnitudeGreaterThan(v1, mag);
}

bool vec3_AlmostTheSame(vec3 v1, vec3 v2)
{
    return equalf(v1.x, v2.x) && equalf(v1.y, v2.y) && equalf(v1.z, v2.z);
}

vec3 vec3_Random(f32 min, f32 max)
{
    return (vec3){
        .x = Random_F32_Range(min, max),
        .y = Random_F32_Range(min, max),
        .z = Random_F32_Range(min, max),
    };
}

vec3 vec3_RandomInUnitDisc(void)
{
    vec3 vec;

    do {
        vec = (vec3){
            .x = Random_F32_Range(-1, 1),
            .y = Random_F32_Range(-1, 1),
            .z = 0,
        };

        if (vec3_DotProduct(vec, vec) < 1) {
            break;
        }
    } while (1);

    return vec;
}

vec3 vec3_RandomInUnitSphere(void)
{
    vec3 vec;

    do {
        vec = vec3_Random(-1, 1);

        if (vec3_DotProduct(vec, vec) < 1) {
            break;
        }
    } while (1);

    return vec;
}

vec3 vec3_RandomOnUnitSphere(void)
{
    return vec3_Normalize(vec3_RandomInUnitSphere());
}

vec3 vec3_RandomInHemisphere(vec3 normal)
{
    vec3 inUnitSphere = vec3_RandomInUnitSphere();

    if (vdot(inUnitSphere, normal) > 0.0f) {
        return inUnitSphere;
    } else {
        return vmul(-1.0f, inUnitSphere);
    }
}

vec3 vec3_Reflect(vec3 vec, vec3 normal)
{
    const f32  vecDotNormal      = vec3_DotProduct(vec, normal);
    const vec3 angleScaledNormal = vec3_MultiplyScalar(normal, 2.0f * vecDotNormal);
    return vec3_Subtract(vec, angleScaledNormal);
}

vec3 vec3_Refract(vec3 vec, vec3 normal, f32 refractRatio)
{
    f32  cosTheta   = fminf(vec3_DotProduct(vec3_MultiplyScalar(vec, -1), normal), 1.0f);
    vec3 vecOutPerp = vec3_MultiplyScalar(vec3_Add(vec, vec3_MultiplyScalar(normal, cosTheta)), refractRatio);
    vec3 vecOutPara = vec3_MultiplyScalar(normal, -sqrtf(fabsf(1.0f - vec3_DotProduct(vecOutPerp, vecOutPerp))));
    return vec3_Add(vecOutPerp, vecOutPara);
}

/* --- Vec4 Functions --- */

vec4 vec4_Add(vec4 v1, vec4 v2)
{
    return (vec4){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
        .w = v1.w + v2.w,
    };
}

vec4 vec4_Subtract(vec4 v1, vec4 v2)
{
    return (vec4){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
        .w = v1.w - v2.w,
    };
}

vec4 vec4_MultiplyScalar(vec4 vec, f32 scalar)
{
    return (vec4){
        .x = scalar * vec.x,
        .y = scalar * vec.y,
        .z = scalar * vec.z,
        .w = scalar * vec.w,
    };
}

vec4 vec4_MultiplyScalarR(f32 scalar, vec4 vec)
{
    return vec4_MultiplyScalar(vec, scalar);
}

vec4 vec4_MultiplyComponents(vec4 v1, vec4 v2)
{
    return (vec4){
        .x = v1.x * v2.x,
        .y = v1.y * v2.y,
        .z = v1.z * v2.z,
        .w = v1.w * v2.w,
    };
}

vec4 vec4_DivideScalar(vec4 vec, f32 scalar)
{
    return vec4_MultiplyScalar(vec, 1.0f / scalar);
}

vec4 vec4_DivideComponents(vec4 vdividend, vec4 vdivisor)
{
    return (vec4){
        .x = vdividend.x / vdivisor.x,
        .y = vdividend.y / vdivisor.y,
        .z = vdividend.z / vdivisor.z,
        .w = vdividend.w / vdivisor.w,
    };
}

f32 vec4_DotProduct(vec4 v1, vec4 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

f32 vec4_MagnitudeSquared(vec4 vec)
{
    return vec4_DotProduct(vec, vec);
}

f32 vec4_Magnitude(vec4 vec)
{
    return sqrtf(vec4_MagnitudeSquared(vec));
}

vec4 vec4_Normalize(vec4 vec)
{
    return vec4_DivideScalar(vec, vec4_Magnitude(vec));
}

bool vec4_CompareMagnitudeEqual(vec4 v1, f32 mag)
{
    const f32 v1mag = vec4_MagnitudeSquared(v1);
    return equalf(v1mag, mag * mag);
}

bool vec4_CompareMagnitudeEqualR(f32 mag, vec4 v1)
{
    return vec4_CompareMagnitudeEqual(v1, mag);
}

bool vec4_CompareMagnitudeGreaterThan(vec4 v1, f32 mag)
{
    const f32 v1mag = vec4_MagnitudeSquared(v1);
    return v1mag > mag;
}

bool vec4_CompareMagnitudeGreaterThanR(f32 mag, vec4 v1)
{
    return vec4_CompareMagnitudeGreaterThan(v1, mag);
}

bool vec4_AlmostTheSame(vec4 v1, vec4 v2)
{
    return equalf(v1.x, v2.x) && equalf(v1.y, v2.y) && equalf(v1.z, v2.z) && equalf(v1.w, v2.w);
}

/* --- Scalar --- */

f32 scalar_Multiply(f32 x, f32 y)
{
    return x * y;
}

bool scalar_AlmostTheSame(f32 x, f32 y)
{
    return equalf(x, y);
}
