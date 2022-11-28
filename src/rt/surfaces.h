#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "math/vec.h"
#include "rt/ray.h"

typedef union {
    struct {
        point3 min;
        point3 max;
    };

    point3 bounds[2];
} BoundingBox;

typedef enum {
    SURFACE_SPHERE,
    SURFACE_TRIANGLE,
} SurfaceType;

typedef struct {
    point3 c;
    f32    r;
} Sphere;

typedef struct {
    point3 pos;
    point3 norm;
    point2 tex;
} Vertex;

typedef struct {
    Vertex vtx[3];
} Triangle;

typedef struct {
    SurfaceType type;

    union {
        Sphere   sphere;
        Triangle triangle;
    };
} Surface;

Surface Surface_Sphere_Make(point3 center, f32 radius);
Surface Surface_Triangle_Make(Vertex v0, Vertex v1, Vertex v2);

bool Sphere_BoundedBy(Sphere* sphere, BoundingBox* box);
bool Triangle_BoundedBy(Triangle* tri, BoundingBox* box);

bool Sphere_HitAt(Sphere* sphere, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);
bool Triangle_HitAt(Triangle* tri, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);

Triangle Triangle_MakeSimple(point3 v0, point3 v1, point3 v2);
