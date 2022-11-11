#include "surfaces.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common/math.h"

Sphere Sphere_Make(point3 center, f32 radius)
{
    return (Sphere){
        .c = center,
        .r = radius,
    };
}

bool Sphere_BoundedBy(const Sphere* sphere, BoundingBox* box)
{
    const point3 center  = sphere->c;
    const f32    epsilon = 0.001f;
    const vec3   vRad    = (vec3){
             .x = sphere->r + epsilon,
             .y = sphere->r + epsilon,
             .z = sphere->r + epsilon,
    };

    box->min = vsub(center, vRad);
    box->max = vadd(center, vRad);

    return true;
}

static vec2 Sphere_MapUV(const point3 onSphere)
{
    f32 theta = acosf(clampf(-onSphere.y, -0.999f, 0.999f));
    f32 phi   = atan2f(-onSphere.z, onSphere.x) + PI;

    // assert(!isnanf(theta));
    // assert(!isnanf(phi));

    return (vec2){
        .x = phi / (2 * PI),
        .y = theta / PI,
    };
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

        hit->tIntersect    = tIntersect;
        hit->position      = Ray_At(ray, tIntersect);
        vec3 outwardNormal = vdiv(vsub(hit->position, sphere->c), sphere->r);
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);
        hit->uv = Sphere_MapUV(outwardNormal);

        return true;
    }
}

Triangle Triangle_MakeSimple(point3 v0, point3 v1, point3 v2)
{
    vec3 edge1         = vsub(v1, v0);
    vec3 edge2         = vsub(v2, v0);
    vec3 defaultNormal = vcross(edge1, edge2);

    return (Triangle){
        .v[0] = {.pos = v0, .norm = defaultNormal, .tex = {0.0f, 0.0f}},
        .v[1] = {.pos = v1, .norm = defaultNormal, .tex = {0.0f, 0.0f}},
        .v[2] = {.pos = v2, .norm = defaultNormal, .tex = {0.0f, 0.0f}}
    };
}

Triangle Triangle_Make(Vertex v0, Vertex v1, Vertex v2)
{
    return (Triangle){
        .v = {v0, v1, v2}
    };
}

bool Triangle_BoundedBy(const Triangle* tri, BoundingBox* box)
{
    const f32 epsilon = 0.001f;

    for (Axis axis = AXIS_X; axis < AXIS_Z + 1; axis++) {
        box->min.v[axis] = minf(minf(tri->v[0].pos.v[axis], tri->v[1].pos.v[axis]), tri->v[2].pos.v[axis]) - epsilon;
        box->max.v[axis] = maxf(maxf(tri->v[0].pos.v[axis], tri->v[1].pos.v[axis]), tri->v[2].pos.v[axis]) + epsilon;
    }

    return true;
}

bool Triangle_HitAt(const Triangle* tri, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
{
    const f32 epsilon = 0.0001f;

    vec3 edge1 = vsub(tri->v[1].pos, tri->v[0].pos);
    vec3 edge2 = vsub(tri->v[2].pos, tri->v[0].pos);

    vec3 h = vcross(ray->dir, edge2);
    f32  a = vdot(edge1, h);

    if (fabsf(a) < epsilon) {
        // ray is parallel to the triangle plane
        return false;
    }

    vec3 s = vsub(ray->origin, tri->v[0].pos);
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
        hit->position   = Ray_At(ray, t);
        hit->tIntersect = t;

        f32 coeff          = 1.0f - u - v;
        hit->uv            = vsum(vmul(tri->v[0].tex, coeff), vmul(tri->v[1].tex, u), vmul(tri->v[2].tex, v));
        vec3 outwardNormal = vsum(vmul(tri->v[0].norm, coeff), vmul(tri->v[1].norm, u), vmul(tri->v[2].norm, v));
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}
