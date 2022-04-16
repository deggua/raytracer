#include "ray.h"

Ray Ray_Make(point3 origin, vec3 dir, f32 time)
{
    return (Ray){
        .origin = origin,
        .dir    = dir,
        .time   = time,
        .cache = {
            .invDir = {
                .x = 1.0f / dir.x,
                .y = 1.0f / dir.y,
                .z = 1.0f / dir.z,
            },
            .originDivDir = {
                .x = origin.x / dir.x,
                .y = origin.y / dir.y,
                .z = origin.z / dir.z,
            },
        },
    };
}

point3 Ray_At(const Ray* ray, f32 dist)
{
    return vadd(ray->origin, vmul(ray->dir, dist));
}
