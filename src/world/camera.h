#pragma once

#include "math/vec.h"
#include "rt/ray.h"

typedef struct {
    point3 origin;
    point3 bottomLeftCorner;
    vec3   horizontal;
    vec3   vertical;
    vec3   u, v, w;
    f32    lensRadius;
    f32    timeStart, timeEnd;
} Camera;

Camera* Camera_New(
    point3 lookFrom,
    point3 lookTo,
    vec3   vup,
    f32    aspectRatio,
    f32    vertFov,
    f32    aperature,
    f32    focusDist,
    f32    timeStart,
    f32    timeEnd);

Ray Camera_GetRay(const Camera* cam, f32 u, f32 v);
