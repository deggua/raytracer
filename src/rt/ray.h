#pragma once

#include "common/common.h"
#include "common/vec.h"

typedef struct {
    point3 origin;
    vec3   dir;
    f32    time;
    struct {
        vec3 invDir;
        vec3 originDivDir;
    } cache;
} Ray;

Ray    Ray_Make(point3 origin, vec3 dir, f32 time);
point3 Ray_At(const Ray* ray, f32 dist);
