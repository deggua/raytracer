#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "gfx/color.h"
#include "gfx/primitives.h"
#include "gfx/utils.h"
#include "object/hitinfo.h"

/* ---- LAMBERT ---- */

Lambert Lambert_Make(Color albedo)
{
    return (Lambert){
        .albedo = albedo,
    };
}

bool Lambert_Bounce(const Lambert* lambert, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut)
{
    (void)rayIn;

    Vec3 scattered = vadd(hit->unitNormal, Vec3_RandomOnUnitSphere());

    if (viszero(scattered)) {
        scattered = hit->unitNormal;
    }

    *color  = lambert->albedo;
    *rayOut = (Ray){
        .origin = hit->position,
        .dir    = scattered,
        .time   = rayIn->time,
    };

    return true;
}

/* ---- METAL ---- */

Metal Metal_Make(Color albedo, float fuzz)
{
    return (Metal){
        .albedo = albedo,
        .fuzz   = fuzz,
    };
}

bool Metal_Bounce(const Metal* metal, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut)
{
    // TODO: do we need to do the face fix thing on the hit normal? I'm guessing we do because otherwise the reflect
    // may be be backwards, atm we always make the normal point against
    Vec3 reflected = vreflect(vunit(rayIn->dir), hit->unitNormal);
    Vec3 fuzz      = vmul(Vec3_RandomOnUnitSphere(), metal->fuzz);
    Ray  scattered = {
         .origin = hit->position,
         .dir    = vadd(reflected, fuzz),
         .time   = rayIn->time,
    };

    *color  = metal->albedo;
    *rayOut = scattered;

    // NOTE: this calculation requires the normal to point against the ray, and is to catch scattered rays entering the
    // object
    return vdot(rayOut->dir, hit->unitNormal) > 0.0f;
}

/* ---- DIELECTRIC ---- */

Dielectric Dielectric_Make(Color albedo, float refractiveIndex)
{
    return (Dielectric){
        .albedo         = albedo,
        .refactiveIndex = refractiveIndex,
    };
}

static float reflectance(float cosine, float refractiveIndex)
{
    float r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
    r0       = r0 * r0;
    return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

bool Dielectric_Bounce(const Dielectric* diel, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut)
{
    *color                = diel->albedo;
    float refractionRatio = hit->frontFace ? (1.0f / diel->refactiveIndex) : diel->refactiveIndex;

    Vec3  normRayDir = vunit(rayIn->dir);
    float cosTheta   = fminf(vdot(vmul(normRayDir, -1), hit->unitNormal), 1.0f);
    float sinTheta   = sqrtf(1.0f - cosTheta * cosTheta);

    bool cannotRefract = refractionRatio * sinTheta > 1.0f;
    Vec3 bounce;
    if (cannotRefract || reflectance(cosTheta, refractionRatio) > randf()) {
        bounce = vreflect(normRayDir, hit->unitNormal);
    } else {
        bounce = vrefract(normRayDir, hit->unitNormal, refractionRatio);
    }

    *rayOut = (Ray){
        .origin = hit->position,
        .dir    = bounce,
        .time   = rayIn->time,
    };

    return true;
}
