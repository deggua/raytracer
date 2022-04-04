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

        case SURFACE_TRIANGLE:
        case SURFACE_NULL: {
            return false;
        } break;
    }
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

        case SURFACE_TRIANGLE:
        case SURFACE_NULL: {
            return false;
        } break;
    }
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

        case MATERIAL_NULL: {
            return false;
        } break;
    }
}

static int CompareBB_X(const void* a, const void* b)
{
    const ObjectBB* bbA = *(ObjectBB**)a;
    const ObjectBB* bbB = *(ObjectBB**)b;

    if (bbA->box.pMin.x < bbB->box.pMin.x) {
        return -1;
    } else if (bbA->box.pMin.x > bbB->box.pMin.x) {
        return 1;
    } else {
        return 0;
    }
}

static int CompareBB_Y(const void* a, const void* b)
{
    const ObjectBB* bbA = *(ObjectBB**)a;
    const ObjectBB* bbB = *(ObjectBB**)b;

    if (bbA->box.pMin.y < bbB->box.pMin.y) {
        return -1;
    } else if (bbA->box.pMin.y > bbB->box.pMin.y) {
        return 1;
    } else {
        return 0;
    }
}

static int CompareBB_Z(const void* a, const void* b)
{
    const ObjectBB* bbA = *(ObjectBB**)a;
    const ObjectBB* bbB = *(ObjectBB**)b;

    if (bbA->box.pMin.z < bbB->box.pMin.z) {
        return -1;
    } else if (bbA->box.pMin.z > bbB->box.pMin.z) {
        return 1;
    } else {
        return 0;
    }
}

void ObjectBB_SortX(ObjectBB* sort[], size_t len)
{
    qsort(sort, len, sizeof(ObjectBB*), CompareBB_X);
}

void ObjectBB_SortY(ObjectBB* sort[], size_t len)
{
    qsort(sort, len, sizeof(ObjectBB*), CompareBB_Y);
}

void ObjectBB_SortZ(ObjectBB* sort[], size_t len)
{
    qsort(sort, len, sizeof(ObjectBB*), CompareBB_Z);
}
