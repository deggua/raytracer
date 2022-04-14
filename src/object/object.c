#include "object.h"

#include <stdlib.h>

#include "object/materials.h"

bool Surface_HitAt(const Surface* surface, const Ray* ray, float tMin, float tMax, HitInfo* hit)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_HitAt(&surface->sphere, ray, tMin, tMax, hit);
        } break;

        case SURFACE_TRIANGLE: {
            return Triangle_HitAt(&surface->triangle, ray, tMin, tMax, hit);
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

        case SURFACE_TRIANGLE: {
            return Triangle_BoundedBy(&surface->triangle, box);
        } break;
    }

    __builtin_unreachable();
}

bool Material_Bounce(const Material* material, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut)
{
    switch (material->type) {
        case MATERIAL_DIFFUSE: {
            return Diffuse_Bounce(&material->diffuse, rayIn, hit, color, rayOut);
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
