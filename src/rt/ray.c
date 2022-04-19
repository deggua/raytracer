#include "ray.h"

Ray Ray_Make(point3 origin, vec3 dir)
{
    return (Ray) {
        .origin = origin,
        .dir    = dir,
        .cache  = {
            .invDir = {
                1.0f / dir.x,
                1.0f / dir.y,
                1.0f / dir.z,
            },
            .originDivDir = {
                origin.x / dir.x,
                origin.y / dir.y,
                origin.z / dir.z,
            },
        },
    };
}

point3 Ray_At(const Ray* ray, f32 dist)
{
    return vadd(ray->origin, vmul(ray->dir, dist));
}

void HitInfo_SetFaceNormal(HitInfo* hit, const Ray* ray, vec3 outwardNormal)
{
    hit->frontFace  = vdot(ray->dir, outwardNormal) < 0;
    hit->unitNormal = hit->frontFace ? outwardNormal : vmul(outwardNormal, -1.0f);
}
