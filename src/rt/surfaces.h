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

// TODO: it would be more efficient to store a triangle as a pointer to a Mesh object
// with indices for each vertex/vertex normal/texture coord as an index into the vector
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

Sphere Sphere_Make(point3 center, f32 radius);
bool   Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box);
bool   Sphere_HitAt(const Sphere* sphere, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);

Triangle Triangle_Make(Vertex v0, Vertex v1, Vertex v2);
Triangle Triangle_MakeSimple(point3 v0, point3 v1, point3 v2);
bool     Triangle_BoundedBy(const Triangle* tri, BoundingBox* box);
bool     Triangle_HitAt(const Triangle* tri, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);
