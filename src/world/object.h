#pragma once

#include <stdbool.h>

#include "rt/materials.h"
#include "rt/ray.h"
#include "rt/surfaces.h"

typedef struct {
#ifndef NDEBUG
    const char* obj_name;
#endif
    Material* material;
    Surface   surface; // TODO: should Object just hold a ptr to surface?, also should Surface even have the concept of
    // position? we could save 8 bytes per copy of object, and reduce the memory cost of having the
    // same mesh instantiated many times significantly, but would have a computation overhead at
    // runtime (transform to/from object space), doing it in a pre-pass in Scene_Prepare could save us
    // from having to do that, but would require separate copies of Surface which negates the savings
    // per duplicated mesh. Logically either seems fine, but currently we have no real notion of object
    // space once a mesh is converted to objects
} Object;

bool Surface_BoundedBy(Surface* surface, BoundingBox* box);
bool Surface_HitAt(Surface* surface, Ray* ray, f32 tMin, f32 tMax, HitInfo* hitInfo);
bool Material_Bounce(
    Material* material,
    Ray*      rayIn,
    HitInfo*  hit,
    Color*    colorSurface,
    Color*    colorEmitted,
    Ray*      rayOut);
