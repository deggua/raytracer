#pragma once

#include "surfaces.h"

#include <stdbool.h>

#include "object/hitinfo.h"
#include "gfx/primitives.h"

typedef enum {
    SURFACE_NULL = 0,
    SURFACE_SPHERE,
    SURFACE_TRIANGLE,
    SURFACE_MOVING_SPHERE,
} SurfaceType;

typedef struct {
    Point3 center;
    float radius;
} Sphere;

typedef struct {
    Point3 (*centerPath)(float time);
    float radius;
} MovingSphere;

typedef struct {
    Point3 vertices[3];
} Triangle;

typedef struct {
    SurfaceType type;
    union {
        Sphere sphere;
        MovingSphere movingSphere;
        Triangle triangle;
    };
} Surface;

Sphere Sphere_Make(Point3 center, float radius);
bool Sphere_HitAt(const Sphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit);

MovingSphere MovingSphere_Make(Point3 (*centerPath)(float time), float radius);
bool MovingSphere_HitAt(const MovingSphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit);
