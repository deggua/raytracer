#pragma once

#include <stdbool.h>

#include "gfx/primitives.h"

#include "object/hitinfo.h"
#include "object/surfaces.h"
#include "object/materials.h"

typedef struct {
    Material material;
    Surface surface;
} Object;

bool Surface_HitAt(const Surface* surface, const Ray* ray, float tMin, float tMax, HitInfo* hitInfo);
bool Material_Bounce(const Material* material, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);
