#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "common/math.h"
#include "common/random.h"
#include "common/vec.h"
#include "gfx/color.h"

/* ---- DIFFUSE ---- */

Material Material_Diffuse_Make(const Texture* tex)
{
    return (Material)
    {
        .type = MATERIAL_DIFFUSE,
        .diffuse = {
            .albedo = tex,
        },
    };
}

bool Material_Diffuse_Bounce(
    const Material_Diffuse* diffuse,
    const Ray*              rayIn,
    const HitInfo*          hit,
    Color*                  colorSurface,
    Color*                  colorEmitted,
    Ray*                    rayOut)
{
    (void)rayIn;

    vec3 target    = vadd(hit->position, vec3_RandomInHemisphere(hit->unitNormal));
    vec3 scattered = vsub(target, hit->position);

    if (vequ(scattered, 0.0f)) {
        scattered = hit->unitNormal;
    }

    *colorSurface = Texture_ColorAt(diffuse->albedo, hit->uv);
    *colorEmitted = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};
    *rayOut       = Ray_Make(hit->position, scattered);

    return true;
}

/* ---- METAL ---- */

Material Material_Metal_Make(const Texture* tex, f32 fuzz)
{
    return (Material){
        .type = MATERIAL_METAL,
        .metal = {
            .albedo = tex,
            .fuzz   = fuzz,
        },
    };
}

bool Material_Metal_Bounce(
    const Material_Metal* metal,
    const Ray*            rayIn,
    const HitInfo*        hit,
    Color*                colorSurface,
    Color*                colorEmitted,
    Ray*                  rayOut)
{
    // TODO: do we need to do the face fix thing on the hit normal? I'm guessing we do because otherwise the reflect
    // may be be backwards, atm we always make the normal point against
    vec3 reflected = vec3_Reflect(vnorm(rayIn->dir), hit->unitNormal);
    vec3 fuzz      = vmul(vec3_RandomOnUnitSphere(), metal->fuzz);

    *colorSurface = Texture_ColorAt(metal->albedo, hit->uv);
    *colorEmitted = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};
    *rayOut       = Ray_Make(hit->position, vadd(reflected, fuzz));

    // NOTE: this calculation requires the normal to point against the ray, and is to catch scattered rays entering the
    // object
    return vdot(rayOut->dir, hit->unitNormal) > 0.0f;
}

/* ---- DIELECTRIC ---- */

Material Material_Dielectric_Make(const Texture* tex, f32 refractiveIndex)
{
    return (Material){
        .type = MATERIAL_DIELECTRIC,
        .dielectric = {
            .albedo         = tex,
            .refactiveIndex = refractiveIndex,
        },
    };
}

static f32 reflectance(f32 cosine, f32 refractiveIndex)
{
    f32 r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
    r0     = r0 * r0;
    return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

bool Material_Dielectric_Bounce(
    const Material_Dielectric* diel,
    const Ray*                 rayIn,
    const HitInfo*             hit,
    Color*                     colorSurface,
    Color*                     colorEmitted,
    Ray*                       rayOut)
{
    *colorSurface       = Texture_ColorAt(diel->albedo, hit->uv);
    *colorEmitted       = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};
    f32 refractionRatio = hit->frontFace ? (1.0f / diel->refactiveIndex) : diel->refactiveIndex;

    vec3 normRayDir = vnorm(rayIn->dir);
    f32  cosTheta   = fminf(vdot(vmul(normRayDir, -1), hit->unitNormal), 1.0f);
    f32  sinTheta   = sqrtf(1.0f - cosTheta * cosTheta);

    bool cannotRefract = refractionRatio * sinTheta > 1.0f;
    vec3 bounce;

    if (cannotRefract || reflectance(cosTheta, refractionRatio) > Random_Normal_f32()) {
        bounce = vec3_Reflect(normRayDir, hit->unitNormal);
    } else {
        bounce = vec3_Refract(normRayDir, hit->unitNormal, refractionRatio);
    }

    *rayOut = Ray_Make(hit->position, bounce);

    return true;
}

/* ---- DIFFUSE LIGHT ---- */

Material Material_DiffuseLight_Make(const Texture* tex, f32 brightness)
{
    return (Material){
        .type = MATERIAL_DIFFUSE_LIGHT,
        .diffuseLight = {
            .albedo     = tex,
            .brightness = brightness,
        },
    };
}

bool Material_DiffuseLight_Bounce(
    const Material_DiffuseLight* diffuseLight,
    const Ray*                   rayIn,
    const HitInfo*               hit,
    Color*                       colorSurface,
    Color*                       colorEmitted,
    Ray*                         rayOut)
{
    (void)rayIn;
    (void)hit;
    (void)colorSurface;
    (void)rayOut;

    *colorEmitted = Color_Brighten(Texture_ColorAt(diffuseLight->albedo, hit->uv), diffuseLight->brightness);

    return false;
}

/* ---- TEST ---- */

Material Material_Test_Make(const Texture* tex)
{
    return (Material){
        .type = MATERIAL_TEST,
        .test = {
            .albedo = tex,
        },
    };
}

bool Material_Test_Bounce(
    const Material_Test* test,
    const Ray*           rayIn,
    const HitInfo*       hit,
    Color*               colorSurface,
    Color*               colorEmitted,
    Ray*                 rayOut)
{
    *colorSurface = Texture_ColorAt(test->albedo, hit->uv);
    *colorEmitted = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};

    vec3 normRayDir      = vnorm(rayIn->dir);
    f32  relativeAngle01 = fabsf(vdot(normRayDir, hit->unitNormal));

    const f32 transmitFactor = 0.5f;
    const f32 scatterAmount  = 2.0f;

    if (relativeAngle01 + Random_Normal_f32() > 2.0f * transmitFactor) {
        // if the impact was almost perpendicular let's refract the ray through the surface with scattering
        vec3 refract = vec3_Refract(vnorm(rayIn->dir), hit->unitNormal, 2.4f);
        vec3 scatter = vadd(refract, vec3_RandomInHemisphere(refract));

        *rayOut = Ray_Make(hit->position, scatter);
    } else {
        // if the impact was oblique, scatter the ray proportional to the impact angle
        // head on => high scattering
        // oblique => low scattering
        vec3 reflect    = vec3_Reflect(normRayDir, hit->unitNormal);
        vec3 scattering = vmul(relativeAngle01 * relativeAngle01 * scatterAmount, vec3_RandomInHemisphere(reflect));
        vec3 combined   = vadd(reflect, scattering);

        *rayOut = Ray_Make(hit->position, combined);
    }

    return true;
}

/* ---- SKYBOX ---- */

Material Material_Skybox_Make(const Skybox* skybox)
{
    return (Material){
        .type = MATERIAL_SKYBOX,
        .skybox = {
            .skybox = skybox,
        },
    };
}

bool Material_Skybox_Bounce(
    const Material_Skybox* skybox,
    const Ray*             rayIn,
    const HitInfo*         hit,
    Color*                 colorSurface,
    Color*                 colorEmitted,
    Ray*                   rayOut)
{
    vec3 target    = vadd(hit->position, vec3_RandomInHemisphere(hit->unitNormal));
    vec3 scattered = vsub(target, hit->position);

    if (vequ(scattered, 0.0f)) {
        scattered = hit->unitNormal;
    }

    Color skyboxColor = Skybox_ColorAt(skybox->skybox, rayIn->dir);
    *colorSurface     = skyboxColor;
    *colorEmitted     = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};

    *rayOut = Ray_Make(hit->position, scattered);

    return true;
}
