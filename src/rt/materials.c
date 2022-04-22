#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "common/common.h"
#include "common/math.h"
#include "common/random.h"
#include "common/vec.h"
#include "gfx/color.h"

/* ---- DIFFUSE ---- */

Diffuse Diffuse_Make(const Texture* tex)
{
    return (Diffuse) {
        .albedo = tex,
    };
}

bool Diffuse_Bounce(const Diffuse* diffuse, const Ray* rayIn, const HitInfo* hit, Color* colorSurface,
                    Color* colorEmitted, Ray* rayOut)
{
    (void)rayIn;

#if 1
    vec3 target = vadd(hit->position, vec3_RandomInHemisphere(hit->unitNormal));
    vec3 scattered = vsub(target, hit->position);
#else
    vec3 scattered = vadd(hit->unitNormal, vec3_RandomOnUnitSphere());
#endif

    if (vequ(scattered, 0.0f)) {
        scattered = hit->unitNormal;
    }

    *colorSurface  = Texture_ColorAt(diffuse->albedo, hit->uv);
    *colorEmitted = (Color) {.r = 0.0f, .g = 0.0f, .b = 0.0f};
    *rayOut = Ray_Make(hit->position, scattered);

    return true;
}

/* ---- METAL ---- */

Metal Metal_Make(const Texture* tex, f32 fuzz)
{
    return (Metal) {
        .albedo = tex,
        .fuzz   = fuzz,
    };
}

bool Metal_Bounce(const Metal* metal, const Ray* rayIn, const HitInfo* hit, Color* colorSurface, Color* colorEmitted,
                  Ray* rayOut)
{
    // TODO: do we need to do the face fix thing on the hit normal? I'm guessing we do because otherwise the reflect
    // may be be backwards, atm we always make the normal point against
    vec3 reflected = vec3_Reflect(vnorm(rayIn->dir), hit->unitNormal);
    vec3 fuzz      = vmul(vec3_RandomOnUnitSphere(), metal->fuzz);

    *colorSurface  = Texture_ColorAt(metal->albedo, hit->uv);
    *colorEmitted = (Color) {.r = 0.0f, .g = 0.0f, .b = 0.0f};
    *rayOut = Ray_Make(hit->position, vadd(reflected, fuzz));

    // NOTE: this calculation requires the normal to point against the ray, and is to catch scattered rays entering the
    // object
    return vdot(rayOut->dir, hit->unitNormal) > 0.0f;
}

/* ---- DIELECTRIC ---- */

Dielectric Dielectric_Make(const Texture* tex, f32 refractiveIndex)
{
    return (Dielectric) {
        .albedo         = tex,
        .refactiveIndex = refractiveIndex,
    };
}

static f32 reflectance(f32 cosine, f32 refractiveIndex)
{
    f32 r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
    r0     = r0 * r0;
    return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

bool Dielectric_Bounce(const Dielectric* diel, const Ray* rayIn, const HitInfo* hit, Color* colorSurface,
                       Color* colorEmitted, Ray* rayOut)
{
    *colorSurface = Texture_ColorAt(diel->albedo, hit->uv);
    *colorEmitted = (Color) {.r = 0.0f, .g = 0.0f, .b = 0.0f};
    f32 refractionRatio = hit->frontFace ? (1.0f / diel->refactiveIndex) : diel->refactiveIndex;

    vec3 normRayDir = vnorm(rayIn->dir);
    f32  cosTheta   = fminf(vdot(vmul(normRayDir, -1), hit->unitNormal), 1.0f);
    f32  sinTheta   = sqrtf(1.0f - cosTheta * cosTheta);

    bool cannotRefract = refractionRatio * sinTheta > 1.0f;
    vec3 bounce;

    if (cannotRefract || reflectance(cosTheta, refractionRatio) > RNG_Random()) {
        bounce = vec3_Reflect(normRayDir, hit->unitNormal);
    } else {
        bounce = vec3_Refract(normRayDir, hit->unitNormal, refractionRatio);
    }

    *rayOut = Ray_Make(hit->position, bounce);

    return true;
}

DiffuseLight DiffuseLight_Make(const Texture* tex, f32 brightness)
{
    return (DiffuseLight) {
        .albedo = tex,
        .brightness = brightness,
    };
}

bool DiffuseLight_Bounce(const DiffuseLight* diffuseLight, const Ray* rayIn, const HitInfo* hit,
                         Color* colorSurface, Color* colorEmitted, Ray* rayOut)
{
    (void)rayIn;
    (void)hit;
    (void)colorSurface;
    (void)rayOut;

    *colorEmitted = Color_Brighten(Texture_ColorAt(diffuseLight->albedo, hit->uv), diffuseLight->brightness);

    return false;
}
