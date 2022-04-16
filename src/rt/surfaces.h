#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common/common.h"
#include "common/vec.h"
#include "rt/hitinfo.h"

typedef union {
    struct {
        point3 min;
        point3 max;
    };
    point3 bounds[2];
} BoundingBox;

typedef enum
{
    SURFACE_SPHERE,
    SURFACE_TRIANGLE,
} SurfaceType;

typedef struct {
    point3 c;
    f32    r;
} Sphere;

typedef struct {
    point3 v[3];
} Triangle;

typedef struct {
    SurfaceType type;
    union {
        Sphere   sphere;
        Triangle triangle;
    };
} Surface;

Sphere Sphere_Make(point3 center, f32 radius);
bool   Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box);
bool   Sphere_HitAt(const Sphere* sphere, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);

Triangle Triangle_Make(point3 v1, point3 v2, point3 v3);
bool     Triangle_BoundedBy(const Triangle* tri, BoundingBox* box);
bool     Triangle_HitAt(const Triangle* tri, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);
