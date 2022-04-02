#pragma once

#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef Vec3 Point3;

#define VEC3_FMT "<%.02f, %.02f, %.02f>"
#define VEC3_ARG(vec) vec.x, vec.y, vec.z

Vec3 Vec3_Add(Vec3 v1, Vec3 v2);
Vec3 Vec3_Sub(Vec3 v1, Vec3 v2);
Vec3 Vec3_Mul(Vec3 vec, float scalar);
Vec3 Vec3_Div(Vec3 vec, float scalar);
float Vec3_Dot(Vec3 v1, Vec3 v2);
float Vec3_Mag(Vec3 vec);
Vec3 Vec3_Normalize(Vec3 vec);
Vec3 Vec3_Cross(Vec3 v1, Vec3 v2);
Vec3 Vec3_Random(float min, float max);
Vec3 Vec3_RandomOnUnitSphere();
bool Vec3_IsZero(Vec3 vec);
Vec3 Vec3_Reflect(Vec3 vec, Vec3 normal);
Vec3 Vec3_Refract(Vec3 vec, Vec3 normal, float refractRatio);
Vec3 Vec3_RandomInUnitDisc();

#define GETOVERLOAD(IGNORE1, IGNORE2, IGNORE3, INGORE4, IGNORE5, NAME, ...) NAME

#define V3ADD_2(v1, v2)                 Vec3_Add(v1, v2)
#define V3ADD_3(v1, v2, v3)             V3ADD_2(v1, V3ADD_2(v2, v3))
#define V3ADD_4(v1, v2, v3, v4)         V3ADD_2(V3ADD_2(v1, v2), V3ADD_2(v3, v4))
#define V3ADD_5(v1, v2, v3, v4, v5)     V3ADD_2(V3ADD_4(v1, v2, v3, v4), v5)
#define V3ADD_6(v1, v2, v3, v4, v5, v6) V3ADD_2(V3ADD_4(v1, v2, v3, v4), V3ADD_2(v5, v6))

#define V3SUB_2(v1, v2)                 Vec3_Sub(v1, v2)
#define V3SUB_3(v1, v2, v3)             V3SUB_2(v1, V3ADD_2(v2, v3))
#define V3SUB_4(v1, v2, v3, v4)         V3SUB_2(v1, V3ADD_3(v2, v3, v4))
#define V3SUB_5(v1, v2, v3, v4, v5)     V3SUB_2(v1, V3ADD_4(v2, v3, v4, v5))
#define V3SUB_6(v1, v2, v3, v4, v5, v6) V3SUB_2(v1, V3ADD_5(v2, v3, v4, v5, v6))

#define vadd(vec1, ...) GETOVERLOAD(__VA_ARGS__, V3ADD_6, V3ADD_5, V3ADD_4, V3ADD_3, V3ADD_2, IGNORE)(vec1, __VA_ARGS__)
#define vsub(vec1, ...) GETOVERLOAD(__VA_ARGS__, V3SUB_6, V3SUB_5, V3SUB_4, V3SUB_3, V3SUB_2, IGNORE)(vec1, __VA_ARGS__)
#define vmul Vec3_Mul
#define vdiv Vec3_Div
#define vdot Vec3_Dot
#define vmag Vec3_Mag
#define vunit Vec3_Normalize
#define vcross Vec3_Cross
#define vrand Vec3_Random
#define viszero Vec3_IsZero
#define vreflect Vec3_Reflect
#define vrefract Vec3_Refract

typedef struct {
    Point3 origin;
    Vec3 dir;
    float time;
} Ray;

Ray Ray_Make(Point3 origin, Vec3 dir, float time);
Point3 Ray_At(const Ray* ray, float dist);

