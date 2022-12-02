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
    SURFACE_PLANE,
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
    vec3   normal;
    point3 point;
} Plane;

typedef struct {
    SurfaceType type;

    union {
        Sphere   sphere;
        Triangle triangle;
        Plane    plane;
    };
} Surface;

Surface Surface_Sphere_Make(point3 center, f32 radius);
Surface Surface_Triangle_Make(Vertex v0, Vertex v1, Vertex v2);
Surface Surface_Plane_Make(point3 point, vec3 normal);

BoundingBox Sphere_BoundingBox(Sphere* sphere);
BoundingBox Triangle_BoundingBox(Triangle* tri);
BoundingBox Plane_BoundingBox(Plane* plane);

bool Sphere_Bounded(void);
bool Triangle_Bounded(void);
bool Plane_Bounded(void);

bool Sphere_HitAt(Sphere* sphere, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);
bool Triangle_HitAt(Triangle* tri, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit);
bool Plane_HitAt(Plane* plane, Ray* ray, f32 t_min, f32 t_max, HitInfo* hit);

Triangle Triangle_MakeSimple(point3 v0, point3 v1, point3 v2);
