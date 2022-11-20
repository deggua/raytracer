#include "ray.h"

Ray Ray_Make(point3 origin, vec3 dir)
{
    vec3 unit = {1.0f, 1.0f, 1.0f};

    return (Ray) {
        .origin = origin,
        .dir    = dir,
        .cache  = {
            .invDir = vdiv(unit, dir),
            .originDivDir = vdiv(origin, dir),
        },
    };
}

point3 Ray_At(in Ray* ray, f32 dist)
{
    return vadd(ray->origin, vmul(ray->dir, dist));
}

void HitInfo_SetFaceNormal(out HitInfo* hit, in Ray* ray, vec3 outwardNormal)
{
    hit->frontFace  = vdot(ray->dir, outwardNormal) < 0;
    hit->unitNormal = hit->frontFace ? outwardNormal : vmul(outwardNormal, -1.0f);
}
