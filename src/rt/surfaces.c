#include "surfaces.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common/math.h"

Sphere Sphere_Make(point3 center, f32 radius)
{
    return (Sphere) {
        .c = center,
        .r = radius,
    };
}

bool Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box)
{
    const point3 center  = sphere->c;
    const f32    epsilon = 0.001f;
    const vec3   vRad    = (vec3) {
        .x = sphere->r + epsilon,
        .y = sphere->r + epsilon,
        .z = sphere->r + epsilon,
    };

    box->min = vsub(center, vRad);
    box->max = vadd(center, vRad);

    return true;
}

bool Sphere_HitAt(const Sphere* sphere, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
{
    vec3 dist         = vsub(ray->origin, sphere->c);
    f32  polyA        = vdot(ray->dir, ray->dir);
    f32  halfPolyB    = vdot(dist, ray->dir);
    f32  polyC        = vdot(dist, dist) - sphere->r * sphere->r;
    f32  discriminant = halfPolyB * halfPolyB - polyA * polyC;

    if (discriminant < 0.0f) {
        return false;
    } else {
        // TODO: it doesn't seem necesarily true to me that the smallest root is going to be root1, what if polyA is
        // negative? Prove this to myself or change the computation
        f32 root1 = (-halfPolyB - sqrtf(discriminant)) / polyA;
        f32 root2 = (-halfPolyB + sqrtf(discriminant)) / polyA;

        f32 tIntersect;

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
        vec3 outwardNormal = vdiv(vsub(hit->position, sphere->c), sphere->r);
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}

Triangle Triangle_Make(point3 v1, point3 v2, point3 v3)
{
    return (Triangle) {
        .v = {v1, v2, v3},
    };
}

bool Triangle_BoundedBy(const Triangle* tri, BoundingBox* box)
{
    const f32 epsilon = 0.001f;

    for (Axis axis = AXIS_X; axis < AXIS_Z + 1; axis++) {
        box->min.v[axis] = minf(minf(tri->v[0].v[axis], tri->v[1].v[axis]), tri->v[2].v[axis]) - epsilon;
        box->max.v[axis] = maxf(maxf(tri->v[0].v[axis], tri->v[1].v[axis]), tri->v[2].v[axis]) + epsilon;
    }

    return true;
}

bool Triangle_HitAt(const Triangle* tri, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
{
    const f32 epsilon = 0.0001f;

    vec3 edge1 = vsub(tri->v[1], tri->v[0]);
    vec3 edge2 = vsub(tri->v[2], tri->v[0]);

    vec3 h = vcross(ray->dir, edge2);
    f32  a = vdot(edge1, h);

    if (fabsf(a) < epsilon) {
        // ray is parallel to the triangle plane
        return false;
    }

    vec3 s = vsub(ray->origin, tri->v[0]);
    f32  u = vdot(s, h) / a;

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    vec3 q = vcross(s, edge1);
    f32  v = vdot(ray->dir, q) / a;

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    f32 t = vdot(edge2, q) / a;

    if (t > tMax || t < tMin) {
        return false;
    } else {
        hit->position      = Ray_At(ray, t);
        hit->tIntersect    = t;
        vec3 outwardNormal = vnorm(vcross(edge1, edge2));
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);
        return true;
    }
}
