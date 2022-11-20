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
    vec3   unitNormal;
    vec2   uv;
    f32    tIntersect;
    bool   frontFace;
} HitInfo;

Ray    Ray_Make(point3 origin, vec3 dir);
point3 Ray_At(in Ray* ray, f32 dist);

void HitInfo_SetFaceNormal(HitInfo* hit, out Ray* ray, vec3 outwardNormal);
