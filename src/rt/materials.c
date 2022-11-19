#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "gfx/color.h"
#include "math/distribution.h"
#include "math/math.h"
#include "math/random.h"
#include "math/vec.h"

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
        .diffuse_light = {
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

/* ---- Disney Diffuse ---- */

static inline vec3 HalfVector(vec3 w_in, vec3 w_out)
{
    return vnorm(vadd(w_in, w_out));
}

Material Material_Disney_Diffuse_Make(const Texture* albedo, f32 roughness, f32 subsurface)
{
    return (Material){
        .type = MATERIAL_DISNEY_DIFFUSE,
        .disney_diffuse = {
            .albedo = albedo,
            .roughness = roughness,
            .subsurface = subsurface,
        },
    };
}

static inline f32 F_D90(vec3 w_out, vec3 half_vector, f32 roughness)
{
    return 0.5f + 2.0f * roughness * POWF(vdot(half_vector, w_out), 2);
}

static inline f32 F_D(vec3 w, vec3 shading_normal, f32 f_d90)
{
    return 1.0f + (f_d90 - 1.0f) * POWF(1.0f - fabsf(vdot(shading_normal, w)), 5);
}

static inline Color BRDF_BaseDiffuse(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
{
    vec3 half_vector = HalfVector(w_in, w_out);
    f32  f_d90       = F_D90(w_out, half_vector, roughness);
    f32  attenuation = (1.0f / PI32) * F_D(w_in, shading_normal, f_d90) * F_D(w_out, shading_normal, f_d90);
    return vmul(base_color, attenuation);
}

static inline f32 F_SS90(vec3 w_out, vec3 half_vector, f32 roughness)
{
    return roughness * POWF(vdot(half_vector, w_out), 2);
}

static inline f32 F_SS(vec3 w, vec3 shading_normal, f32 f_ss90)
{
    return 1.0 + (f_ss90 - 1.0f) * POWF(1.0f - fabsf(vdot(shading_normal, w)), 5);
}

static inline Color BRDF_Subsurface(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
{
    vec3 half_vector = HalfVector(w_in, w_out);
    f32  f_ss90      = F_SS90(w_out, half_vector, roughness);

    f32 abs_n_w_in  = fabsf(vdot(shading_normal, w_in));
    f32 abs_n_w_out = fabsf(vdot(shading_normal, w_out));

    f32 f_ss_w_in  = F_SS(w_in, shading_normal, f_ss90);
    f32 f_ss_w_out = F_SS(w_out, shading_normal, f_ss90);

    f32 attenuation = (1.25f / PI32) * (f_ss_w_in * f_ss_w_out * (1.0f / (abs_n_w_in + abs_n_w_out) - 0.5f) + 0.5f);

    return vmul(base_color, attenuation);
}

static inline Color
BRDF_Diffuse(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness, f32 subsurface)
{
    Color brdf_base_diffuse = BRDF_BaseDiffuse(base_color, w_in, w_out, shading_normal, roughness);
    Color brdf_subsurface   = BRDF_Subsurface(base_color, w_in, w_out, shading_normal, roughness);
    return vlerp(brdf_base_diffuse, brdf_subsurface, subsurface);
}

bool Material_Disney_Diffuse_Bounce(
    in Material_Disney_Diffuse* mat,
    in Ray*                     ray_in,
    in HitInfo*                 hit,
    out Color*                  surface_color,
    out Color*                  emitted_color,
    out Ray*                    ray_out)
{
    // generate sample vector
    vec3 ray_dir_out = Distribution_CosWeightedHemisphere_Sample(hit->unitNormal);

    vec3  shading_normal = hit->unitNormal;
    Color albedo         = Texture_ColorAt(mat->albedo, hit->uv);

    // w_in should point from the surface to the observer
    vec3 w_in  = vmul(-1.0f, vnorm(ray_in->dir));
    vec3 w_out = ray_dir_out;

    Color brdf_diffuse = BRDF_Diffuse(albedo, w_in, w_out, shading_normal, mat->roughness, mat->subsurface);

    // since we omit the cos term in the BRDF, we don't need the cos term in the PDF
    // so we just multiply by PI32 (essentially)
    f32 pdf      = Distribution_CosWeightedHemisphere_PDF_InvNoCos();
    brdf_diffuse = vmul(brdf_diffuse, pdf);

    *ray_out       = Ray_Make(hit->position, ray_dir_out);
    *surface_color = brdf_diffuse;
    *emitted_color = COLOR_BLACK;

    return true;
}

/* ---- Disney Metal ---- */

Material Material_Disney_Metal_Make(in Texture* albedo, f32 roughness, f32 anistropic)
{
    return (Material){
        .type = MATERIAL_DISNEY_METAL,
        .disney_metal = {
            .albedo = albedo,
            .roughness = roughness,
            .anistropic = anistropic,
        },
    };
}

#if 0
// Schlick approximation of the Fresnel reflection
// TODO: It's not obvious that base_color should be used like this to me, esp. reading the paper
static inline Color F_m(Color base_color, vec3 half_vector, vec3 w_out)
{
    Color white = COLOR_WHITE;
    return vadd(base_color, vmul(vsub(white, base_color), POWF(1.0f - fabsf(vdot(half_vector, w_out)), 5)));
}

static inline Color D_m()


bool Material_Disney_Metal_Bounce(
    in Material_Disney_Metal* mat,
    in Ray*                   ray_in,
    in HitInfo*               hit,
    out Color*                surface_color,
    out Color*                emitted_color,
    out Ray*                  ray_out)
{
    return true;
}
#endif
