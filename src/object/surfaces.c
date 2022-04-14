#include "surfaces.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gfx/utils.h"
#include "object/hitinfo.h"

Sphere Sphere_Make(Point3 center, float radius)
{
    return (Sphere){
        .c = center,
        .r = radius,
    };
}

bool Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box)
{
    const Point3 center  = sphere->c;
    const float  epsilon = 0.001f;
    const Vec3   vRad    = (Vec3){
             .x = sphere->r + epsilon,
             .y = sphere->r + epsilon,
             .z = sphere->r + epsilon,
    };

    box->min = vsub(center, vRad);
    box->max = vadd(center, vRad);

    return true;
}

bool Sphere_HitAt(const Sphere* sphere, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    Vec3  dist         = vsub(ray->origin, sphere->c);
    float polyA        = vdot(ray->dir, ray->dir);
    float halfPolyB    = vdot(dist, ray->dir);
    float polyC        = vdot(dist, dist) - sphere->r * sphere->r;
    float discriminant = halfPolyB * halfPolyB - polyA * polyC;

    if (discriminant < 0.0f) {
        return false;
    } else {
        // TODO: it doesn't seem necesarily true to me that the smallest root is going to be root1, what if polyA is
        // negative? Prove this to myself or change the computation
        float root1 = (-halfPolyB - sqrtf(discriminant)) / polyA;
        float root2 = (-halfPolyB + sqrtf(discriminant)) / polyA;

        float tIntersect;
        if (tMin < root1 && root1 < tMax) {
            tIntersect = root1;
        } else if (tMin < root2 && root2 < tMax) {
            tIntersect = root2;
        } else {
            return false;
        }

        hit->tIntersect = tIntersect;
        hit->position   = Ray_At(ray, tIntersect);

        // hit->unitNormal = v3div(v3sub(hit->position, sphere->center), sphere->radius);
        // TODO: I don't think we should do this here, we don't necessarily need this info at hit time, so why calculate
        // it
        Vec3 outwardNormal = vdiv(vsub(hit->position, sphere->c), sphere->r);
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}

Triangle Triangle_Make(Point3 v1, Point3 v2, Point3 v3)
{
    return (Triangle){
        .v = {v1, v2, v3},
    };
}

bool Triangle_BoundedBy(const Triangle* tri, BoundingBox* box)
{
    const float epsilon = 0.001f;

    for (VecAxis axis = VEC_X; axis < VEC_LAST; axis++) {
        box->min.e[axis] = minf(minf(tri->v[0].e[axis], tri->v[1].e[axis]), tri->v[2].e[axis]) - epsilon;
        box->max.e[axis] = maxf(maxf(tri->v[0].e[axis], tri->v[1].e[axis]), tri->v[2].e[axis]) + epsilon;
    }

    return true;
}

bool Triangle_HitAt(const Triangle* tri, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    const float epsilon = 0.0001f;

    Vec3 edge1 = vsub(tri->v[1], tri->v[0]);
    Vec3 edge2 = vsub(tri->v[2], tri->v[0]);

    Vec3  h = vcross(ray->dir, edge2);
    float a = vdot(edge1, h);

    if (fabsf(a) < epsilon) {
        // ray is parallel to the triangle plane
        return false;
    }

    Vec3  s = vsub(ray->origin, tri->v[0]);
    float u = vdot(s, h) / a;
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    Vec3  q = vcross(s, edge1);
    float v = vdot(ray->dir, q) / a;

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = vdot(edge2, q) / a;
    if (t > tMax || t < tMin) {
        return false;
    } else {
        hit->position      = Ray_At(ray, t);
        hit->tIntersect    = t;
        Vec3 outwardNormal = vunit(vcross(edge1, edge2));
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);
        return true;
    }
}
