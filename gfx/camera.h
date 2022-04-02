#pragma once

#include "primitives.h"

typedef struct {
    Point3 origin;
    Point3 bottomLeftCorner;
    Vec3 horizontal;
    Vec3 vertical;
    Vec3 u, v, w;
    float lensRadius;
    float timeStart, timeEnd;
} Camera;

Camera Camera_Make(
    Point3 lookFrom,
    Point3 lookTo,
    Vec3   vup,
    float  aspectRatio,
    float  vertFov,
    float  aperature,
    float  focusDist,
    float  timeStart,
    float  timeEnd);
Ray Camera_GetRay(Camera* cam, float u, float v);
