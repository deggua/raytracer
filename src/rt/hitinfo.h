#pragma once

#include <stdbool.h>

#include "common/common.h"
#include "common/vec.h"
#include "rt/ray.h"

typedef struct {
    point3 position;
    vec3   unitNormal; // TODO: consider making this not a unit normal by default if, more often than not, it doesn't
                       // matter
    f32  tIntersect;
    bool frontFace;
} HitInfo;

void HitInfo_SetFaceNormal(HitInfo* hit, const Ray* ray, vec3 outwardNormal);
