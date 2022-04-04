#include "surfaces.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gfx/utils.h"
#include "object/hitinfo.h"

Sphere Sphere_Make(Point3 center, float radius)
{
    return (Sphere){
        .center = center,
        .radius = radius,
    };
}

bool Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box)
{
    const Point3 center  = sphere->center;
    const float  epsilon = 0.001f;
    const Vec3   vRad    = (Vec3){
             .x = sphere->radius + epsilon,
             .y = sphere->radius + epsilon,
             .z = sphere->radius + epsilon,
    };

    box->pMin = vsub(center, vRad);
    box->pMax = vadd(center, vRad);

    return true;
}

bool Sphere_HitAt(const Sphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    Vec3  dist         = vsub(ray->origin, sphere->center);
    float polyA        = vdot(ray->dir, ray->dir);
    float halfPolyB    = vdot(dist, ray->dir);
    float polyC        = vdot(dist, dist) - sphere->radius * sphere->radius;
    float discriminant = halfPolyB * halfPolyB - polyA * polyC;

    if (discriminant < 0.0f) {
        return false;
    } else {
        // TODO: it doesn't seem necesarily true to me that the smallest root is going to be root1, what if polyA is
        // negative? Prove this to myself or change the computation
        float root1 = (-halfPolyB - sqrtf(discriminant)) / polyA;
        float root2 = (-halfPolyB + sqrtf(discriminant)) / polyA;

        float tIntersect;
        if (within(root1, tMin, tMax)) {
            tIntersect = root1;
        } else if (within(root2, tMin, tMax)) {
            tIntersect = root2;
        } else {
            return false;
        }

        hit->tIntersect = tIntersect;
        hit->position   = Ray_At(ray, tIntersect);

        // hit->unitNormal = v3div(v3sub(hit->position, sphere->center), sphere->radius);
        // TODO: I don't think we should do this here, we don't necessarily need this info at hit time, so why calculate
        // it
        Vec3 outwardNormal = vdiv(vsub(hit->position, sphere->center), sphere->radius);
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}

MovingSphere MovingSphere_Make(Point3 (*centerPath)(float time), float radius)
{
    return (MovingSphere){
        .centerPath = centerPath,
        .radius     = radius,
    };
}

bool MovingSphere_BoundedBy(const MovingSphere* sphere, BoundingBox* box)
{
    (void)sphere;
    (void)box;
    return false;
}

bool MovingSphere_HitAt(const MovingSphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    Point3 sphereCenter = sphere->centerPath(ray->time);
    Vec3   dist         = vsub(ray->origin, sphereCenter);
    float  polyA        = vdot(ray->dir, ray->dir);
    float  halfPolyB    = vdot(dist, ray->dir);
    float  polyC        = vdot(dist, dist) - sphere->radius * sphere->radius;
    float  discriminant = halfPolyB * halfPolyB - polyA * polyC;

    if (discriminant < 0.0f) {
        return false;
    } else {
        // TODO: it doesn't seem necesarily true to me that the smallest root is going to be root1, what if polyA is
        // negative? Prove this to myself or change the computation
        float root1 = (-halfPolyB - sqrtf(discriminant)) / polyA;
        float root2 = (-halfPolyB + sqrtf(discriminant)) / polyA;

        float tIntersect;
        if (within(root1, tMin, tMax)) {
            tIntersect = root1;
        } else if (within(root2, tMin, tMax)) {
            tIntersect = root2;
        } else {
            return false;
        }

        hit->tIntersect = tIntersect;
        hit->position   = Ray_At(ray, tIntersect);

        // hit->unitNormal = v3div(v3sub(hit->position, sphere->center), sphere->radius);
        // TODO: I don't think we should do this here, we don't necessarily need this info at hit time, so why calculate
        // it
        Vec3 outwardNormal = vdiv(vsub(hit->position, sphereCenter), sphere->radius);
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}
