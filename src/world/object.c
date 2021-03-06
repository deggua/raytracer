#include "object.h"

#include <stdlib.h>

#include "common/common.h"
#include "rt/materials.h"

bool Surface_HitAt(const Surface* surface, const Ray* ray, f32 tMin, f32 tMax, HitInfo* hit)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_HitAt(&surface->sphere, ray, tMin, tMax, hit);
        }
        break;

        case SURFACE_TRIANGLE: {
            return Triangle_HitAt(&surface->triangle, ray, tMin, tMax, hit);
        }
        break;
    }

    OPTIMIZE_UNREACHABLE;
}

bool Surface_BoundedBy(const Surface* surface, BoundingBox* box)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_BoundedBy(&surface->sphere, box);
        }
        break;

        case SURFACE_TRIANGLE: {
            return Triangle_BoundedBy(&surface->triangle, box);
        }
        break;
    }

    OPTIMIZE_UNREACHABLE;
}

bool Material_Bounce(const Material* material, const Ray* rayIn, const HitInfo* hit, Color* colorSurface,
                     Color* colorEmitted, Ray* rayOut)
{
    switch (material->type) {
        case MATERIAL_DIFFUSE: {
            return Diffuse_Bounce(&material->diffuse, rayIn, hit, colorSurface, colorEmitted, rayOut);
        }
        break;

        case MATERIAL_METAL: {
            return Metal_Bounce(&material->metal, rayIn, hit, colorSurface, colorEmitted, rayOut);
        }
        break;

        case MATERIAL_DIELECTRIC: {
            return Dielectric_Bounce(&material->dielectric, rayIn, hit, colorSurface, colorEmitted, rayOut);
        }
        break;

        case MATERIAL_DIFFUSE_LIGHT: {
            return DiffuseLight_Bounce(&material->diffuseLight, rayIn, hit, colorSurface, colorEmitted, rayOut);
        }
        break;
    }

    OPTIMIZE_UNREACHABLE;
}
