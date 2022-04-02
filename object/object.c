#include "object.h"

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
