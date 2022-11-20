#include "object.h"

#include <stdlib.h>

#include "rt/materials.h"

bool Surface_HitAt(in Surface* surface, in Ray* ray, f32 tMin, f32 tMax, out HitInfo* hit)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_HitAt(&surface->sphere, ray, tMin, tMax, hit);
        } break;

        case SURFACE_TRIANGLE: {
            return Triangle_HitAt(&surface->triangle, ray, tMin, tMax, hit);
        } break;
    }

    OPTIMIZE_UNREACHABLE;
}

bool Surface_BoundedBy(in Surface* surface, out BoundingBox* box)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_BoundedBy(&surface->sphere, box);
        } break;

        case SURFACE_TRIANGLE: {
            return Triangle_BoundedBy(&surface->triangle, box);
        } break;
    }

    OPTIMIZE_UNREACHABLE;
}

bool Material_Bounce(
    in Material* material,
    in Ray*      rayIn,
    in HitInfo*  hit,
    out Color*   colorSurface,
    out Color*   colorEmitted,
    out Ray*     rayOut)
{
    switch (material->type) {
        case MATERIAL_DIFFUSE: {
            return Material_Diffuse_Bounce(&material->diffuse, rayIn, hit, colorSurface, colorEmitted, rayOut);
        } break;

        case MATERIAL_METAL: {
            return Material_Metal_Bounce(&material->metal, rayIn, hit, colorSurface, colorEmitted, rayOut);
        } break;

        case MATERIAL_DIELECTRIC: {
            return Material_Dielectric_Bounce(&material->dielectric, rayIn, hit, colorSurface, colorEmitted, rayOut);
        } break;

        case MATERIAL_DIFFUSE_LIGHT: {
            return Material_DiffuseLight_Bounce(
                &material->diffuse_light,
                rayIn,
                hit,
                colorSurface,
                colorEmitted,
                rayOut);
        } break;

        case MATERIAL_SKYBOX: {
            return Material_Skybox_Bounce(&material->skybox, rayIn, hit, colorSurface, colorEmitted, rayOut);
        } break;

        case MATERIAL_DISNEY_DIFFUSE: {
            return Material_Disney_Diffuse_Bounce(
                &material->disney_diffuse,
                rayIn,
                hit,
                colorSurface,
                colorEmitted,
                rayOut);
        } break;

        case MATERIAL_DISNEY_METAL: {
            return Material_Disney_Metal_Bounce(
                &material->disney_metal,
                rayIn,
                hit,
                colorSurface,
                colorEmitted,
                rayOut);
        } break;

        case MATERIAL_DISNEY_GLASS: {
            assert(false);
        } break;

        case MATERIAL_DISNEY_CLEARCOAT: {
            assert(false);
        } break;

        case MATERIAL_DISNEY_SHEEN: {
            assert(false);
        } break;

        case MATERIAL_DISNEY_BSDF: {
            assert(false);
        } break;
    }

    OPTIMIZE_UNREACHABLE;
}
