#pragma once

#include <stdbool.h>

#include "gfx/primitives.h"
#include "object/hitinfo.h"
#include "object/materials.h"
#include "object/surfaces.h"

typedef struct {
    Material material;
    Surface  surface;
} Object;

typedef struct {
    BoundingBox   box;
    const Object* obj;
} ObjectBB;

bool Surface_BoundedBy(const Surface* surface, BoundingBox* box);
bool Surface_HitAt(const Surface* surface, const Ray* ray, float tMin, float tMax, HitInfo* hitInfo);
bool Material_Bounce(const Material* material, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);

void ObjectBB_Sort(ObjectBB* sort[], size_t len, VecAxis axis);
