#include "surfaces.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "math/math.h"

/* ---- Sphere ---- */

Surface Surface_Sphere_Make(point3 center, f32 radius)
{
    return (Surface) {
        .type = SURFACE_SPHERE,
        .sphere = {
            .c = center,
            .r = radius,
        },
    };
}

BoundingBox Sphere_BoundingBox(Sphere* sphere)
{
    point3 center = sphere->c;

    vec3 vRad = (vec3){
        .x = sphere->r + RT_EPSILON,
        .y = sphere->r + RT_EPSILON,
        .z = sphere->r + RT_EPSILON,
    };

    return (BoundingBox){
        .min = vsub(center, vRad),
        .max = vadd(center, vRad),
    };
}

bool Sphere_Bounded(void)
{
    return true;
}

intern inline vec2 Sphere_MapUV(point3 onSphere)
{
    f32 theta = acosf(clampf(-onSphere.y, -0.999f, 0.999f));
    f32 phi   = atan2f(-onSphere.z, onSphere.x) + PI32;

    return (vec2){
        .x = phi / (2 * PI32),
        .y = theta / PI32,
    };
}

bool Sphere_HitAt(Sphere* sphere, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
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

/* ---- Triangle ---- */

Triangle Triangle_MakeSimple(point3 v0, point3 v1, point3 v2)
{
    vec3 edge1         = vsub(v1, v0);
    vec3 edge2         = vsub(v2, v0);
    vec3 defaultNormal = vcross(edge1, edge2);

    return (Triangle){
        .vtx[0] = {.pos = v0, .norm = defaultNormal, .tex = {0.0f, 0.0f}},
        .vtx[1] = {.pos = v1, .norm = defaultNormal, .tex = {0.0f, 0.0f}},
        .vtx[2] = {.pos = v2, .norm = defaultNormal, .tex = {0.0f, 0.0f}}
    };
}

Surface Surface_Triangle_Make(Vertex v0, Vertex v1, Vertex v2)
{
    return (Surface) {
        .type = SURFACE_TRIANGLE,
        .triangle = {
            .vtx = {v0, v1, v2},
        },
    };
}

BoundingBox Triangle_BoundingBox(Triangle* tri)
{
    BoundingBox box;

    for (Axis axis = AXIS_X; axis <= AXIS_Z; axis++) {
        box.min.elem[axis]
            = minf(minf(tri->vtx[0].pos.elem[axis], tri->vtx[1].pos.elem[axis]), tri->vtx[2].pos.elem[axis])
              - RT_EPSILON;
        box.max.elem[axis]
            = maxf(maxf(tri->vtx[0].pos.elem[axis], tri->vtx[1].pos.elem[axis]), tri->vtx[2].pos.elem[axis])
              + RT_EPSILON;
    }

    return box;
}

bool Triangle_Bounded(void)
{
    return true;
}

bool Triangle_HitAt(Triangle* tri, Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
{
    vec3 edge1 = vsub(tri->vtx[1].pos, tri->vtx[0].pos);
    vec3 edge2 = vsub(tri->vtx[2].pos, tri->vtx[0].pos);

    vec3 h = vcross(ray->dir, edge2);
    f32  a = vdot(edge1, h);

    if (fabsf(a) < RT_EPSILON) {
        // ray is parallel to the triangle plane
        return false;
    }

    vec3 s = vsub(ray->origin, tri->vtx[0].pos);
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
        hit->uv            = vsum(vmul(tri->vtx[0].tex, coeff), vmul(tri->vtx[1].tex, u), vmul(tri->vtx[2].tex, v));
        vec3 outwardNormal = vsum(vmul(tri->vtx[0].norm, coeff), vmul(tri->vtx[1].norm, u), vmul(tri->vtx[2].norm, v));
        HitInfo_SetFaceNormal(hit, ray, outwardNormal);

        return true;
    }
}

/* ---- Plane ---- */

Surface Surface_Plane_Make(point3 point, vec3 normal)
{
    return (Surface){
        .type  = SURFACE_PLANE,
        .plane = {
            .normal = normal,
            .point  = point,
        },
    };
}

BoundingBox Plane_BoundingBox(Plane* plane)
{
    for (Axis axis = AXIS_X; axis <= AXIS_Z; axis++) {
        Axis next_axis = (axis + 1) % AXIS_W;
        Axis prev_axis = (axis + 2) % AXIS_W;
        if (plane->normal.elem[next_axis] == 0.0f && plane->normal.elem[prev_axis] == 0.0f) {
            BoundingBox box;

            box.min.elem[prev_axis] = -INF;
            box.min.elem[axis]      = -RT_EPSILON;
            box.min.elem[next_axis] = -INF;

            box.max.elem[prev_axis] = INF;
            box.max.elem[axis]      = RT_EPSILON;
            box.max.elem[next_axis] = INF;

            return box;
        }
    }

    return (BoundingBox){
        .min = {-INF, -INF, -INF},
        .max = { INF,  INF,  INF},
    };
}

bool Plane_Bounded(void)
{
    return false;
}

bool Plane_HitAt(Plane* plane, Ray* ray, f32 t_min, f32 t_max, HitInfo* hit)
{
    f32 numerator   = vdot(plane->normal, vsub(plane->point, ray->origin));
    f32 denominator = vdot(plane->normal, ray->dir);

    f32 t_intersect = numerator / denominator;

    if (t_intersect < t_min || t_intersect > t_max) {
        return false;
    } else {
        hit->position   = Ray_At(ray, t_intersect);
        hit->tIntersect = t_intersect;

        vec3 outward_normal = plane->normal;
        HitInfo_SetFaceNormal(hit, ray, outward_normal);

        return true;
    }
}
