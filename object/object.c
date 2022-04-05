#include "object.h"

#include <stdlib.h>

#include "object/materials.h"

bool Surface_HitAt(const Surface* surface, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_HitAt(&surface->sphere, ray, tMin, tMax, hit);
        } break;

        case SURFACE_MOVING_SPHERE: {
            return MovingSphere_HitAt(&surface->movingSphere, ray, tMin, tMax, hit);
        } break;

        case SURFACE_TRIANGLE: {
            return false;
        } break;
    }

    __builtin_unreachable();
}

bool Surface_BoundedBy(const Surface* surface, BoundingBox* box)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_BoundedBy(&surface->sphere, box);
        } break;

        case SURFACE_MOVING_SPHERE: {
            return MovingSphere_BoundedBy(&surface->movingSphere, box);
        } break;

        case SURFACE_TRIANGLE: {
            return false;
        } break;
    }

    __builtin_unreachable();
}

bool Material_Bounce(const Material* material, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut)
{
    switch (material->type) {
        case MATERIAL_LAMBERT: {
            return Lambert_Bounce(&material->lambert, rayIn, hit, color, rayOut);
        } break;

        case MATERIAL_METAL: {
            return Metal_Bounce(&material->metal, rayIn, hit, color, rayOut);
        } break;

        case MATERIAL_DIELECTRIC: {
            return Dielectric_Bounce(&material->dielectric, rayIn, hit, color, rayOut);
        } break;
    }

    __builtin_unreachable();
}

static int CompareBB(const ObjectBB** a, const ObjectBB** b, VecAxis axis)
{
    const ObjectBB* bbA = *a;
    const ObjectBB* bbB = *b;

    if (bbA->box.pMin.e[axis] < bbB->box.pMin.e[axis]) {
        return -1;
    } else if (bbA->box.pMin.e[axis] > bbB->box.pMin.e[axis]) {
        return 1;
    } else {
        return 0;
    }
}

static int CompareBB_X(const void* a, const void* b)
{
    return CompareBB((const ObjectBB**)a, (const ObjectBB**)b, VEC_X);
}

static int CompareBB_Y(const void* a, const void* b)
{
    return CompareBB((const ObjectBB**)a, (const ObjectBB**)b, VEC_Y);
}

static int CompareBB_Z(const void* a, const void* b)
{
    return CompareBB((const ObjectBB**)a, (const ObjectBB**)b, VEC_Z);
}

void ObjectBB_Sort(ObjectBB* sort[], size_t len, VecAxis axis)
{
    switch (axis) {
        case VEC_X: {
            qsort(sort, len, sizeof(ObjectBB*), CompareBB_X);
        } break;

        case VEC_Y: {
            qsort(sort, len, sizeof(ObjectBB*), CompareBB_Y);
        } break;

        case VEC_Z: {
            qsort(sort, len, sizeof(ObjectBB*), CompareBB_Z);
        } break;
    };

    return;
}
