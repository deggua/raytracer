#pragma once

#include "math/vec.h"

typedef struct {
    struct {
        vec3 invDir;
        vec3 originDivDir;
    } cache;

    point3 origin;
    vec3   dir;
} Ray;

typedef struct {
    point3 position;
    // TODO: consider making this not a unit normal by default if, more often than not, it doesn't matter
    vec3 unitNormal;
    vec2 uv;
    f32  tIntersect;
    bool frontFace;
} HitInfo;

Ray    Ray_Make(point3 origin, vec3 dir);
point3 Ray_At(const Ray* ray, f32 dist);

void HitInfo_SetFaceNormal(HitInfo* hit, const Ray* ray, vec3 outwardNormal);
