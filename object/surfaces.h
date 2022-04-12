#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "gfx/primitives.h"
#include "object/hitinfo.h"
#include "surfaces.h"

typedef union {
    struct {
        Point3 min;
        Point3 max;
    };
    Point3 bounds[2];
} BoundingBox;

typedef enum
{
    SURFACE_SPHERE,
    SURFACE_TRIANGLE,
} SurfaceType;

typedef struct {
    Point3 c;
    float  r;
} Sphere;

typedef struct {
    Point3 v[3];
} Triangle;

typedef struct {
    SurfaceType type;
    union {
        Sphere   sphere;
        Triangle triangle;
    };
} Surface;

Sphere Sphere_Make(Point3 center, float radius);
bool   Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box);
bool   Sphere_HitAt(const Sphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit);
