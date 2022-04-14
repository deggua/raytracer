#pragma once

#include <stdbool.h>

#include "gfx/primitives.h"

typedef struct {
    Point3 position;
    Vec3 unitNormal;
    float tIntersect;
    bool frontFace;
} HitInfo;

static inline void HitInfo_SetFaceNormal(HitInfo* hit, const Ray* ray, Vec3 outwardNormal) {
    hit->frontFace = vdot(ray->dir, outwardNormal) < 0;
    hit->unitNormal = hit->frontFace ? outwardNormal : vmul(outwardNormal, -1);
}
