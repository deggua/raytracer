#include "object.h"

#include <stdlib.h>

#include "rt/materials.h"

bool Surface_HitAt(Surface* surface, Ray* ray, f32 t_min, f32 t_max, HitInfo* hit)
{
    switch (surface->type) {
        case SURFACE_SPHERE: {
            return Sphere_HitAt(&surface->sphere, ray, t_min, t_max, hit);
        } break;

        case SURFACE_TRIANGLE: {
            return Triangle_HitAt(&surface->triangle, ray, t_min, t_max, hit);
        } break;
    }

    OPTIMIZE_UNREACHABLE;
}

bool Surface_BoundedBy(Surface* surface, BoundingBox* box)
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
    Material* material,
    Ray*      ray_in,
    HitInfo*  hit,
    Color*    surface_color,
    Color*    emitted_color,
    Ray*      ray_out)
{
    switch (material->type) {
        case MATERIAL_DIFFUSE: {
            return Material_Diffuse_Bounce(&material->diffuse, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_METAL: {
            return Material_Metal_Bounce(&material->metal, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_DIELECTRIC: {
            return Material_Dielectric_Bounce(
                &material->dielectric,
                ray_in,
                hit,
                surface_color,
                emitted_color,
                ray_out);
        } break;

        case MATERIAL_DIFFUSE_LIGHT: {
            return Material_DiffuseLight_Bounce(
                &material->diffuse_light,
                ray_in,
                hit,
                surface_color,
                emitted_color,
                ray_out);
        } break;

        case MATERIAL_SKYBOX: {
            return Material_Skybox_Bounce(&material->skybox, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_DISNEY_DIFFUSE: {
            return Material_Disney_Diffuse_Bounce(
                &material->disney,
                ray_in,
                hit,
                surface_color,
                emitted_color,
                ray_out);
        } break;

        case MATERIAL_DISNEY_METAL: {
            return Material_Disney_Metal_Bounce(&material->disney, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_DISNEY_GLASS: {
            return Material_Disney_Glass_Bounce(&material->disney, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_DISNEY_CLEARCOAT: {
            return Material_Disney_Clearcoat_Bounce(
                &material->disney,
                ray_in,
                hit,
                surface_color,
                emitted_color,
                ray_out);
        } break;

        case MATERIAL_DISNEY_SHEEN: {
            return Material_Disney_Sheen_Bounce(&material->disney, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;

        case MATERIAL_DISNEY_BSDF: {
            return Material_Disney_BSDF_Bounce(&material->disney, ray_in, hit, surface_color, emitted_color, ray_out);
        } break;
    }

    OPTIMIZE_UNREACHABLE;
}
