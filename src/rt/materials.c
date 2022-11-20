#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "gfx/color.h"
#include "math/math.h"
#include "math/random.h"
#include "math/vec.h"

/* ---- DIFFUSE ---- */

Material Material_Diffuse_Make(in Texture* tex)
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
    in Material_Diffuse* diffuse,
    in Ray*              rayIn,
    in HitInfo*          hit,
    out Color*           colorSurface,
    out Color*           colorEmitted,
    out Ray*             rayOut)
{
    (void)rayIn;

    vec3 target    = vadd(hit->position, Random_InHemisphere(hit->unitNormal, 1.0f));
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

Material Material_Metal_Make(in Texture* tex, f32 fuzz)
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
    in Material_Metal* metal,
    in Ray*            rayIn,
    in HitInfo*        hit,
    out Color*         colorSurface,
    out Color*         colorEmitted,
    out Ray*           rayOut)
{
    // TODO: do we need to do the face fix thing on the hit normal? I'm guessing we do because otherwise the reflect
    // may be be backwards, atm we always make the normal point against
    vec3 reflected = vec3_Reflect(vnorm(rayIn->dir), hit->unitNormal);
    vec3 fuzz      = vmul(Random_OnSphere(1.0f), metal->fuzz);

    *colorSurface = Texture_ColorAt(metal->albedo, hit->uv);
    *colorEmitted = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};
    *rayOut       = Ray_Make(hit->position, vadd(reflected, fuzz));

    // NOTE: this calculation requires the normal to point against the ray, and is to catch scattered rays entering the
    // object
    return vdot(rayOut->dir, hit->unitNormal) > 0.0f;
}

/* ---- DIELECTRIC ---- */

Material Material_Dielectric_Make(in Texture* tex, f32 refractiveIndex)
{
    return (Material){
        .type = MATERIAL_DIELECTRIC,
        .dielectric = {
            .albedo         = tex,
            .refactiveIndex = refractiveIndex,
        },
    };
}

intern inline f32 reflectance(f32 cosine, f32 refractiveIndex)
{
    f32 r0 = (1.0f - refractiveIndex) / (1.0f + refractiveIndex);
    r0     = r0 * r0;
    return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

bool Material_Dielectric_Bounce(
    in Material_Dielectric* diel,
    in Ray*                 rayIn,
    in HitInfo*             hit,
    out Color*              colorSurface,
    out Color*              colorEmitted,
    out Ray*                rayOut)
{
    *colorSurface       = Texture_ColorAt(diel->albedo, hit->uv);
    *colorEmitted       = (Color){.r = 0.0f, .g = 0.0f, .b = 0.0f};
    f32 refractionRatio = hit->frontFace ? (1.0f / diel->refactiveIndex) : diel->refactiveIndex;

    vec3 normRayDir = vnorm(rayIn->dir);
    f32  cosTheta   = fminf(vdot(vmul(normRayDir, -1), hit->unitNormal), 1.0f);
    f32  sinTheta   = sqrtf(1.0f - cosTheta * cosTheta);

    bool cannotRefract = refractionRatio * sinTheta > 1.0f;
    vec3 bounce;

    if (cannotRefract || reflectance(cosTheta, refractionRatio) > Random_Unilateral()) {
        bounce = vec3_Reflect(normRayDir, hit->unitNormal);
    } else {
        bounce = vec3_Refract(normRayDir, hit->unitNormal, refractionRatio);
    }

    *rayOut = Ray_Make(hit->position, bounce);

    return true;
}

/* ---- DIFFUSE LIGHT ---- */

Material Material_DiffuseLight_Make(in Texture* tex, f32 brightness)
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
    in Material_DiffuseLight* diffuseLight,
    in Ray*                   rayIn,
    in HitInfo*               hit,
    out Color*                colorSurface,
    out Color*                colorEmitted,
    out Ray*                  rayOut)
{
    (void)rayIn;
    (void)hit;
    (void)colorSurface;
    (void)rayOut;

    *colorEmitted = Color_Brighten(Texture_ColorAt(diffuseLight->albedo, hit->uv), diffuseLight->brightness);

    return false;
}

/* ---- SKYBOX ---- */

Material Material_Skybox_Make(in Skybox* skybox)
{
    return (Material){
        .type = MATERIAL_SKYBOX,
        .skybox = {
            .skybox = skybox,
        },
    };
}

bool Material_Skybox_Bounce(
    in Material_Skybox* skybox,
    in Ray*             rayIn,
    in HitInfo*         hit,
    out Color*          colorSurface,
    out Color*          colorEmitted,
    out Ray*            rayOut)
{
    vec3 target    = vadd(hit->position, Random_InHemisphere(hit->unitNormal, 1.0f));
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

intern inline vec3 HalfVector(vec3 w_in, vec3 w_out)
{
    return vnorm(vadd(w_in, w_out));
}

Material Material_Disney_Diffuse_Make(in Texture* albedo, f32 roughness, f32 subsurface)
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

intern inline f32 F_D90(vec3 w_out, vec3 half_vector, f32 roughness)
{
    return 0.5f + 2.0f * roughness * POWF(vdot(half_vector, w_out), 2);
}

intern inline f32 F_D(vec3 w, vec3 shading_normal, f32 f_d90)
{
    return 1.0f + (f_d90 - 1.0f) * POWF(1.0f - fabsf(vdot(shading_normal, w)), 5);
}

intern inline Color BRDF_BaseDiffuse(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
{
    vec3 half_vector = HalfVector(w_in, w_out);
    f32  f_d90       = F_D90(w_out, half_vector, roughness);
    f32  attenuation = (1.0f / PI32) * F_D(w_in, shading_normal, f_d90) * F_D(w_out, shading_normal, f_d90);
    return vmul(base_color, attenuation);
}

intern inline f32 F_SS90(vec3 w_out, vec3 half_vector, f32 roughness)
{
    return roughness * POWF(vdot(half_vector, w_out), 2);
}

intern inline f32 F_SS(vec3 w, vec3 shading_normal, f32 f_ss90)
{
    return 1.0 + (f_ss90 - 1.0f) * POWF(1.0f - fabsf(vdot(shading_normal, w)), 5);
}

intern inline Color BRDF_Subsurface(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
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

intern inline Color
BRDF_Diffuse(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness, f32 subsurface)
{
    Color brdf_base_diffuse = BRDF_BaseDiffuse(base_color, w_in, w_out, shading_normal, roughness);
    Color brdf_subsurface   = BRDF_Subsurface(base_color, w_in, w_out, shading_normal, roughness);
    return vlerp(brdf_base_diffuse, brdf_subsurface, subsurface);
}

intern inline vec3 CosWeightedHemisphere_Sample(vec3 unit_normal)
{
    f32 epsilon_0 = Random_Unilateral();
    f32 epsilon_1 = Random_Unilateral();

    f32 sin_theta = sqrtf(1.0f - epsilon_0);
    f32 cos_theta = sqrtf(epsilon_0);
    f32 phi       = 2.0f * PI32 * epsilon_1;

    vec3 xyz_normal_space = {cosf(phi) * sin_theta, sinf(phi) * sin_theta, cos_theta};

    basis3 basis = vec3_OrthonormalBasis(unit_normal);
    basis        = (basis3){.x = basis.z, .y = basis.y, .z = basis.x};

    vec3 xyz_world_space = vec3_Reorient(xyz_normal_space, basis);

    return xyz_world_space;
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
    vec3 ray_dir_out = CosWeightedHemisphere_Sample(hit->unitNormal);

    vec3  shading_normal = hit->unitNormal;
    Color albedo         = Texture_ColorAt(mat->albedo, hit->uv);

    // w_in should point from the surface to the observer
    vec3 w_in  = vmul(-1.0f, vnorm(ray_in->dir));
    vec3 w_out = ray_dir_out;

    Color brdf_diffuse = BRDF_Diffuse(albedo, w_in, w_out, shading_normal, mat->roughness, mat->subsurface);

    // since we omit the cos term in the BRDF, we don't need the cos term in the PDF
    // so we just multiply by PI32 (essentially)
    f32 inv_pdf  = PI32;
    brdf_diffuse = vmul(brdf_diffuse, inv_pdf);

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

intern inline Color F_Schlick(Color r_0, f32 cos_theta)
{
    return vadd(r_0, vmul(vsub(vec3_Set(1.0f), r_0), POWF(1.0f - cos_theta, 5)));
}

intern inline f32 D_N(vec3 normal, f32 a_x, f32 a_y)
{
    f32 numerator = 1.0f;
    f32 denominator
        = PI32 * a_x * a_y * POWF((POWF(normal.x / a_x, 2) + POWF(normal.y / a_y, 2) + POWF(normal.z, 2)), 2);
    return numerator / denominator;
}

intern inline f32 Lambda_w(vec3 w, f32 a_x, f32 a_y)
{
    f32 numerator   = sqrtf(1.0f + (POWF(w.x * a_x, 2) + POWF(w.y * a_y, 2)) / POWF(w.z, 2)) - 1.0f;
    f32 denominator = 2.0f;

    return numerator / denominator;
}

intern inline f32 G_1(vec3 w, f32 a_x, f32 a_y)
{
    return 1.0f / (1.0f + Lambda_w(w, a_x, a_y));
}

intern inline f32 G_2(vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 a_x, f32 a_y)
{
    if (vdot(w_out, w_micronormal) < 0 || vdot(w_in, w_micronormal) < 0) {
        return 0.0f;
    } else {
        f32 numerator   = 1.0f;
        f32 denominator = 1.0f + Lambda_w(w_out, a_x, a_y) + Lambda_w(w_in, a_x, a_y);
        return numerator / denominator;
    }
}

intern inline Color BRDF_Disney_Metal(Color base_color, vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 a_x, f32 a_y)
{
    Color f_m = F_Schlick(base_color, vdot(w_out, w_micronormal));
    f32   g_2 = G_2(w_in, w_out, w_micronormal, a_x, a_y);
    f32   g_1 = G_1(w_in, a_x, a_y);

    return vmul(f_m, g_2 / g_1);
}

// See: https://jcgt.org/published/0007/04/01/paper.pdf
// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
intern inline vec3 GGXVNDF_Sample(vec3 Ve, f32 alpha_x, f32 alpha_y, f32 U1, f32 U2)
{
    // Section 3.2: transforming the view direction to the hemisphere configuration
    vec3 Vh = vnorm(vec(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    f32  lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1    = lensq > 0 ? vmul(vec(-Vh.y, Vh.x, 0), (1.0f / sqrtf(lensq))) : vec(1, 0, 0);
    vec3 T2    = vcross(Vh, T1);
    // Section 4.2: parameterization of the projected area
    f32 r   = sqrtf(U1);
    f32 phi = 2.0f * PI32 * U2;
    f32 t1  = r * cosf(phi);
    f32 t2  = r * sinf(phi);
    f32 s   = 0.5f * (1.0f + Vh.z);
    t2      = (1.0f - s) * sqrtf(1.0f - t1 * t1) + s * t2;
    // Section 4.3: reprojection onto hemisphere
    vec3 Nh = vsum(vmul(t1, T1), vmul(t2, T2), vmul(sqrtf(maxf(0.0f, 1.0f - t1 * t1 - t2 * t2)), Vh));
    // Section 3.4: transforming the normal back to the ellipsoid configuration
    vec3 Ne = vnorm(vec(alpha_x * Nh.x, alpha_y * Nh.y, maxf(0.0f, Nh.z)));
    return Ne;
}

bool Material_Disney_Metal_Bounce(
    in Material_Disney_Metal* mat,
    in Ray*                   ray_in,
    in HitInfo*               hit,
    out Color*                surface_color,
    out Color*                emitted_color,
    out Ray*                  ray_out)
{
    // convert material parameters to a_x and a_y
    // TODO: no reason to do this on the fly, could be precomputed
    f32 aspect = sqrtf(1.0f - 0.9f * mat->anistropic);
    f32 a_min  = 0.0001f;
    f32 a_x    = maxf(a_min, POWF(mat->roughness, 2) / aspect);
    f32 a_y    = maxf(a_min, POWF(mat->roughness, 2) * aspect);

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_basis = vec3_OrthonormalBasis(hit->unitNormal);
    normal_basis        = (basis3){.x = normal_basis.z, .y = normal_basis.y, .z = normal_basis.x};

    // TODO: Maybe we should consider making any Ray_Make call emit a normalized direction vector,
    // should be more efficient to do it when creating the ray vs on material bounce
    // (I think, esp. if many materials need it normalized anyway)
    vec3 ray_in_dir = vnorm(vmul(-1.0f, ray_in->dir));
    vec3 w_in       = vec3_Reorient(ray_in_dir, normal_basis);

    f32  u1            = Random_Unilateral();
    f32  u2            = Random_Unilateral();
    vec3 w_micronormal = GGXVNDF_Sample(w_in, a_x, a_y, u1, u2);

    vec3 w_out = vnorm(vec3_Reflect(vmul(-1.0f, w_in), w_micronormal));

    Color albedo = Texture_ColorAt(mat->albedo, hit->uv);
    Color brdf   = BRDF_Disney_Metal(albedo, w_in, w_out, w_micronormal, a_x, a_y);

    *surface_color = brdf;
    *emitted_color = COLOR_BLACK;

    // transform w_out into world space
    basis3 world_basis = (basis3){
        .x = vec(1.0f, 0.0f, 0.0f),
        .y = vec(0.0f, 1.0f, 0.0f),
        .z = vec(0.0f, 0.0f, 1.0f),
    };
    vec3 ray_out_dir = vec3_Reorient(w_out, world_basis);
    *ray_out         = Ray_Make(hit->position, ray_out_dir);

    return true;
}
