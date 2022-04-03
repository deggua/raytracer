#include "primitives.h"

#include <math.h>
#include <stdbool.h>

#include "utils.h"

Vec3 Vec3_Add(Vec3 v1, Vec3 v2)
{
    return (Vec3){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
    };
}

Vec3 Vec3_Sub(Vec3 v1, Vec3 v2)
{
    return (Vec3){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
}

Vec3 Vec3_Mul(Vec3 vec, float scalar)
{
    return (Vec3){
        .x = scalar * vec.x,
        .y = scalar * vec.y,
        .z = scalar * vec.z,
    };
}

Vec3 Vec3_Div(Vec3 vec, float scalar)
{
    return Vec3_Mul(vec, 1.0f / scalar);
}

float Vec3_Dot(Vec3 v1, Vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vec3_Mag(Vec3 vec)
{
    return sqrtf(Vec3_Dot(vec, vec));
}

Vec3 Vec3_Normalize(Vec3 vec)
{
    return Vec3_Div(vec, Vec3_Mag(vec));
}

Vec3 Vec3_Cross(Vec3 v1, Vec3 v2)
{
    return (Vec3){
        .x = (v1.y * v2.z - v1.z * v2.y),
        .y = (v1.z * v2.x - v1.x * v2.z),
        .z = (v1.x * v2.y - v1.y * v2.x),
    };
}

// NOTE: this function's vector distribution is subtly biased (to a cube) if min != 0
Vec3 Vec3_Random(float min, float max)
{
    return (Vec3){
        .x = randrf(min, max),
        .y = randrf(min, max),
        .z = randrf(min, max),
    };
}

Vec3 Vec3_RandomInUnitSphere(void)
{
    Vec3 vec;
    do {
        vec = Vec3_Random(-1, 1);

        if (Vec3_Dot(vec, vec) < 1) {
            break;
        }
    } while (1);

    return vec;
}

Vec3 Vec3_RandomOnUnitSphere(void)
{
    return Vec3_Normalize(Vec3_RandomInUnitSphere());
}

Vec3 Vec3_RandomInUnitDisc(void)
{
    Vec3 vec;

    do {
        vec = (Vec3){
            .x = randrf(-1, 1),
            .y = randrf(-1, 1),
            .z = 0,
        };

        if (Vec3_Dot(vec, vec) < 1) {
            break;
        }
    } while (1);

    return vec;
}

bool Vec3_IsZero(Vec3 vec)
{
    // TODO: would taking the inner product and comparing with epsilon squared be faster?
    const float epsilon = 1e-8;
    return (fabsf(vec.x) < epsilon) && (fabsf(vec.y) < epsilon) && (fabsf(vec.z) < epsilon);
}

Vec3 Vec3_Reflect(Vec3 vec, Vec3 normal)
{
    return Vec3_Sub(vec, Vec3_Mul(Vec3_Mul(normal, Vec3_Dot(vec, normal)), 2.0f));
}

Vec3 Vec3_Refract(Vec3 vec, Vec3 normal, float refractRatio)
{
    float cosTheta   = fminf(Vec3_Dot(Vec3_Mul(vec, -1), normal), 1.0f);
    Vec3  vecOutPerp = Vec3_Mul(Vec3_Add(vec, Vec3_Mul(normal, cosTheta)), refractRatio);
    Vec3  vecOutPara = Vec3_Mul(normal, -sqrtf(fabs(1.0f - Vec3_Dot(vecOutPerp, vecOutPerp))));
    return Vec3_Add(vecOutPerp, vecOutPara);
}

/* --- Ray --- */

Ray Ray_Make(Point3 origin, Vec3 dir, float time)
{
    return (Ray){
        .origin = origin,
        .dir    = dir,
        .time   = time,
    };
}

Point3 Ray_At(const Ray* ray, float dist)
{
    return Vec3_Add(ray->origin, Vec3_Mul(ray->dir, dist));
}
