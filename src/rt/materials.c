#include "materials.h"

#include <math.h>
#include <stdbool.h>

#include "gfx/color.h"
#include "math/math.h"
#include "math/random.h"
#include "math/vec.h"

// TODO: I think we might be able to remove a lot of the uses of fabsf in the Disney BRDFs
// due to our method of computation, it shouldn't be possible for the dot product to be negative

/* ---- DIFFUSE ---- */

Material Material_Diffuse_Make(Texture* tex)
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
    Material_Diffuse* diffuse,
    Ray*              rayIn,
    HitInfo*          hit,
    Color*            colorSurface,
    Color*            colorEmitted,
    Ray*              rayOut)
{
    (void)rayIn;

    vec3 target    = vadd(hit->position, Random_InHemisphere(hit->unitNormal, 1.0f));
    vec3 scattered = vsub(target, hit->position);

    if (vequ(scattered, 0.0f)) {
        scattered = hit->unitNormal;
    }

    *colorSurface = Texture_ColorAt(diffuse->albedo, hit->uv);
    *colorEmitted = COLOR_BLACK;
    *rayOut       = Ray_Make(hit->position, scattered);

    return true;
}

/* ---- METAL ---- */

Material Material_Metal_Make(Texture* tex, f32 fuzz)
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
    Material_Metal* metal,
    Ray*            rayIn,
    HitInfo*        hit,
    Color*          colorSurface,
    Color*          colorEmitted,
    Ray*            rayOut)
{
    // TODO: do we need to do the face fix thing on the hit normal? I'm guessing we do because otherwise the reflect
    // may be be backwards, atm we always make the normal point against
    vec3 reflected = vec3_Reflect(vnorm(rayIn->dir), hit->unitNormal);
    vec3 fuzz      = vmul(Random_OnSphere(1.0f), metal->fuzz);

    *colorSurface = Texture_ColorAt(metal->albedo, hit->uv);
    *colorEmitted = COLOR_BLACK;
    *rayOut       = Ray_Make(hit->position, vadd(reflected, fuzz));

    // NOTE: this calculation requires the normal to point against the ray, and is to catch scattered rays entering the
    // object
    return vdot(rayOut->dir, hit->unitNormal) > 0.0f;
}

/* ---- DIELECTRIC ---- */

Material Material_Dielectric_Make(Texture* tex, f32 refractiveIndex)
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
    Material_Dielectric* diel,
    Ray*                 rayIn,
    HitInfo*             hit,
    Color*               colorSurface,
    Color*               colorEmitted,
    Ray*                 rayOut)
{
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

    *colorSurface = Texture_ColorAt(diel->albedo, hit->uv);
    *colorEmitted = COLOR_BLACK;
    *rayOut       = Ray_Make(hit->position, bounce);

    return true;
}

/* ---- DIFFUSE LIGHT ---- */

Material Material_DiffuseLight_Make(Texture* tex, f32 brightness)
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
    Material_DiffuseLight* diffuseLight,
    Ray*                   rayIn,
    HitInfo*               hit,
    Color*                 colorSurface,
    Color*                 colorEmitted,
    Ray*                   rayOut)
{
    (void)rayIn;
    (void)hit;
    (void)colorSurface;
    (void)rayOut;

    *colorEmitted = Color_Brighten(Texture_ColorAt(diffuseLight->albedo, hit->uv), diffuseLight->brightness);

    return false;
}

/* ---- SKYBOX ---- */

Material Material_Skybox_Make(Skybox* skybox)
{
    return (Material){
        .type = MATERIAL_SKYBOX,
        .skybox = {
            .skybox = skybox,
        },
    };
}

bool Material_Skybox_Bounce(
    Material_Skybox* skybox,
    Ray*             rayIn,
    HitInfo*         hit,
    Color*           colorSurface,
    Color*           colorEmitted,
    Ray*             rayOut)
{
    vec3 target    = vadd(hit->position, Random_InHemisphere(hit->unitNormal, 1.0f));
    vec3 scattered = vsub(target, hit->position);

    if (vequ(scattered, 0.0f)) {
        scattered = hit->unitNormal;
    }

    Color skyboxColor = Skybox_ColorAt(skybox->skybox, rayIn->dir);

    *colorSurface = skyboxColor;
    *colorEmitted = COLOR_BLACK;
    *rayOut       = Ray_Make(hit->position, scattered);

    return true;
}

/* ---- Disney Diffuse ---- */

Material Material_Disney_Diffuse_Make(Texture* albedo, f32 roughness, f32 subsurface)
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

intern inline vec3 HalfVector(vec3 w_in, vec3 w_out)
{
    return vnorm(vadd(w_in, w_out));
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
    return 1.0f + (f_ss90 - 1.0f) * POWF(1.0f - fabsf(vdot(shading_normal, w)), 5);
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
    Material_Disney_Diffuse* mat,
    Ray*                     ray_in,
    HitInfo*                 hit,
    Color*                   surface_color,
    Color*                   emitted_color,
    Ray*                     ray_out)
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

    *surface_color = brdf_diffuse;
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, ray_dir_out);

    return true;
}

/* ---- Disney Metal ---- */

Material Material_Disney_Metal_Make(Texture* albedo, f32 roughness, f32 anistropic)
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

intern inline Color Fresnel_Schlick_Chromatic(Color r_0, f32 cos_theta)
{
    return vadd(r_0, vmul(vsub(vec3_Set(1.0f), r_0), POWF(1.0f - cos_theta, 5)));
}

intern inline f32 D_m(vec3 normal, f32 a_x, f32 a_y)
{
    f32 numerator = 1.0f;
    f32 denominator
        = PI32 * a_x * a_y * POWF((POWF(normal.x / a_x, 2) + POWF(normal.y / a_y, 2) + POWF(normal.z, 2)), 2);
    return numerator / denominator;
}

intern inline f32 Lambda_m(vec3 w, f32 a_x, f32 a_y)
{
    f32 numerator   = sqrtf(1.0f + (POWF(w.x * a_x, 2) + POWF(w.y * a_y, 2)) / POWF(w.z, 2)) - 1.0f;
    f32 denominator = 2.0f;

    return numerator / denominator;
}

intern inline f32 G_m1(vec3 w, f32 a_x, f32 a_y)
{
    return 1.0f / (1.0f + Lambda_m(w, a_x, a_y));
}

// NOTE: this (and the other G_*2 functions) are from https://jcgt.org/published/0003/02/03/
// he recommends using the funciton over the product vesion since it's more accurate
// it is slower though since we can't eliminate terms
// TODO: would this be faster without the branch? needs benchmarking
intern inline f32 G_m2(vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 a_x, f32 a_y)
{
#if 0
    f32 chi_pos_w_out = vdot(w_out, w_micronormal) < 0.0f ? 0.0f : 1.0f;
    f32 chi_pos_w_in  = vdot(w_in, w_micronormal) < 0.0f ? 0.0f : 1.0f;

    f32 numerator   = chi_pos_w_out * chi_pos_w_in;
    f32 denominator = 1.0f + Lambda_m(w_out, a_x, a_y) + Lambda_m(w_in, a_x, a_y);

    return numerator / denominator;
#else
    if (vdot(w_out, w_micronormal) < 0 || vdot(w_in, w_micronormal) < 0) {
        return 0.0f;
    } else {
        f32 numerator   = 1.0f;
        f32 denominator = 1.0f + Lambda_m(w_out, a_x, a_y) + Lambda_m(w_in, a_x, a_y);

        return numerator / denominator;
    }
#endif
}

intern inline Color BRDF_Disney_Metal(Color base_color, vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 a_x, f32 a_y)
{
    Color f_m = Fresnel_Schlick_Chromatic(base_color, vdot(w_out, w_micronormal));
    f32   g_2 = G_m2(w_in, w_out, w_micronormal, a_x, a_y);
    f32   g_1 = G_m1(w_in, a_x, a_y);

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

intern inline vec3 Disney_Metal_SampleNormal(vec3 w_in, f32 a_x, f32 a_y)
{
    f32  u1            = Random_Unilateral();
    f32  u2            = Random_Unilateral();
    vec3 w_micronormal = GGXVNDF_Sample(w_in, a_x, a_y, u1, u2);

    return w_micronormal;
}

bool Material_Disney_Metal_Bounce(
    Material_Disney_Metal* mat,
    Ray*                   ray_in,
    HitInfo*               hit,
    Color*                 surface_color,
    Color*                 emitted_color,
    Ray*                   ray_out)
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

    vec3 w_micronormal = Disney_Metal_SampleNormal(w_in, a_x, a_y);

    vec3 w_out = vnorm(vec3_Reflect(vmul(-1.0f, w_in), w_micronormal));

    Color albedo = Texture_ColorAt(mat->albedo, hit->uv);
    Color brdf   = BRDF_Disney_Metal(albedo, w_in, w_out, w_micronormal, a_x, a_y);

    // transform w_out into world space
    basis3 world_basis = (basis3){
        .x = vec(1.0f, 0.0f, 0.0f),
        .y = vec(0.0f, 1.0f, 0.0f),
        .z = vec(0.0f, 0.0f, 1.0f),
    };
    vec3 ray_out_dir = vec3_Reorient(w_out, world_basis);

    *surface_color = brdf;
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, ray_out_dir);

    return true;
}

/* ---- Disney Clearcoat BRDF ---- */

Material Material_Disney_Clearcoat_Make(f32 gloss)
{
    return (Material){
        .type = MATERIAL_DISNEY_CLEARCOAT,
        .disney_clearcoat = {
            .gloss = gloss,
        },
    };
}

intern inline f32 Fresnel_Schlick_Achromatic(vec3 w_out, vec3 w_micronormal)
{
    f32 eta = 1.5f;
    f32 r_0 = POWF(eta - 1.0f, 2) / POWF(eta + 1, 2);

    return r_0 + (1.0f - r_0) * POWF(1.0f - fabsf(vdot(w_micronormal, w_out)), 5);
}

intern inline f32 D_c(vec3 w_micronormal, f32 a_g)
{
    f32 a_g_2 = POWF(a_g, 2);

    f32 numerator   = a_g_2 - 1.0f;
    f32 denominator = 2.0f * PI32 * logf(a_g) * (1.0f + (a_g_2 - 1.0f) * POWF(w_micronormal.z, 2));

    return numerator / denominator;
}

intern inline f32 Lambda_c(vec3 w)
{
    f32 roughness_2 = 0.25f * 0.25f;
    f32 w_l_x_2     = POWF(w.x, 2);
    f32 w_l_y_2     = POWF(w.y, 2);
    f32 w_l_z_2     = POWF(w.z, 2);

    f32 numerator   = sqrtf(1.0f + (roughness_2 * (w_l_x_2 + w_l_y_2)) / w_l_z_2) - 1.0f;
    f32 denominator = 2.0f;

    return numerator / denominator;
}

intern inline f32 G_c1(vec3 w)
{
    return 1.0f / (1.0f + Lambda_c(w));
}

// TODO: would this be faster without the branch? needs benchmarking
intern inline f32 G_c2(vec3 w_in, vec3 w_out, vec3 w_micronormal)
{
#if 0
    f32 chi_pos_w_out = vdot(w_out, w_micronormal) < 0.0f ? 0.0f : 1.0f;
    f32 chi_pos_w_in  = vdot(w_in, w_micronormal) < 0.0f ? 0.0f : 1.0f;

    f32 numerator   = chi_pos_w_out * chi_pos_w_in;
    f32 denominator = 1.0f + Lambda_c(w_out) + Lambda_c(w_in);

    return numerator / denominator;
#else
    if (vdot(w_out, w_micronormal) < 0 || vdot(w_in, w_micronormal) < 0) {
        return 0.0f;
    } else {
        f32 numerator   = 1.0f;
        f32 denominator = 1.0f + Lambda_c(w_out) + Lambda_c(w_in);

        return numerator / denominator;
    }
#endif
}

intern inline f32 BRDF_Disney_Clearcoat(vec3 w_in, vec3 w_out, vec3 w_micronormal, vec3 w_macronormal, f32 a_g)
{
    (void)a_g;
    f32 f_c = Fresnel_Schlick_Achromatic(w_out, w_micronormal);
    // f32 d_c = D_c(w_micronormal, a_g);
    f32 d_c = 1.0f; // due to term cancellation
    f32 g_c = G_c2(w_in, w_out, w_micronormal);

    f32 numerator = f_c * d_c * g_c;
    // TODO: it looks like they use the macronormal, but this might need to be the micronormal
    // f32 denominator = 4.0f * fabsf(vdot(w_macronormal, w_in));
    f32 denominator = fabsf(vdot(w_macronormal, w_in));

    return numerator / denominator;
}

intern inline vec3 Disney_Clearcoat_SampleNormal(f32 a_g)
{
    f32 u_0 = Random_Unilateral();
    f32 u_1 = Random_Unilateral();

    f32 a_g_2 = POWF(a_g, 2);

    f32 cos_h_elev = sqrtf((1.0f - powf(a_g_2, 1.0f - u_0)) / (1.0f - a_g_2));
    f32 h_azi      = 2.0f * PI32 * u_1;

    f32 sin_h_elev = sqrtf(1.0f - POWF(cos_h_elev, 2));
    f32 cos_h_azi  = cosf(h_azi);
    f32 sin_h_azi  = sinf(h_azi);

    return (vec3){
        .x = sin_h_elev * cos_h_azi,
        .y = sin_h_elev * sin_h_azi,
        .z = cos_h_elev,
    };
}

// TODO: might need to swap macronormal for micronormal
intern inline f32 Disney_Clearcoat_SampleNormal_InvPDF(vec3 w_out, vec3 w_micronormal, vec3 w_macronormal, f32 a_g)
{
    (void)a_g;
    // f32 d_c = D_c(w_micronormal, a_g);
    f32 d_c = 1.0f; // due to term cancellation

    f32 numerator   = d_c * fabsf(vdot(w_macronormal, w_micronormal));
    f32 denominator = fabsf(vdot(w_micronormal, w_out));

    return denominator / numerator;
}

bool Material_Disney_Clearcoat_Bounce(
    Material_Disney_Clearcoat* mat,
    Ray*                       ray_in,
    HitInfo*                   hit,
    Color*                     surface_color,
    Color*                     emitted_color,
    Ray*                       ray_out)
{
    // convert material param to a_g
    // TODO: no reason to do this on the fly, could be precomputed
    f32 a_g = (1.0f - mat->gloss) * 0.1f + mat->gloss * 0.001f;

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_basis = vec3_OrthonormalBasis(hit->unitNormal);
    normal_basis        = (basis3){.x = normal_basis.z, .y = normal_basis.y, .z = normal_basis.x};

    // TODO: if we make all rays have normalized direction we can remove the vnorm
    vec3 ray_in_dir = vnorm(vmul(-1.0f, ray_in->dir));
    vec3 w_in       = vec3_Reorient(ray_in_dir, normal_basis);

    vec3 w_macronormal = vec(0.0f, 0.0f, 1.0f);
    vec3 w_micronormal = Disney_Clearcoat_SampleNormal(a_g);

    vec3 w_out = vnorm(vec3_Reflect(vmul(-1.0f, w_in), w_micronormal));

    f32 brdf    = BRDF_Disney_Clearcoat(w_in, w_out, w_micronormal, w_macronormal, a_g);
    f32 inv_pdf = Disney_Clearcoat_SampleNormal_InvPDF(w_out, w_micronormal, w_macronormal, a_g);

    Color brdf_color = vec(brdf * inv_pdf, brdf * inv_pdf, brdf * inv_pdf);

    // transform w_out into world space
    basis3 world_basis = (basis3){
        .x = vec(1.0f, 0.0f, 0.0f),
        .y = vec(0.0f, 1.0f, 0.0f),
        .z = vec(0.0f, 0.0f, 1.0f),
    };
    vec3 ray_out_dir = vec3_Reorient(w_out, world_basis);

    *surface_color = brdf_color;
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, ray_out_dir);

    return true;
}

/* ---- Disney Glass ---- */

Material Material_Disney_Glass_Make(Texture* albedo, f32 roughness, f32 anistropic, f32 eta)
{
    return (Material){
        .type = MATERIAL_DISNEY_GLASS,
        .disney_glass = {
            .albedo = albedo,
            .roughness = roughness,
            .anistropic = anistropic,
            .eta = eta,
        },
    };
}

intern inline f32 Fresnel_Achromatic_DotProducts(f32 micronormal_dot_in, f32 micronormal_dot_out, f32 eta)
{
    f32 r_s = (micronormal_dot_in - eta * micronormal_dot_out) / (micronormal_dot_in + eta * micronormal_dot_out);
    f32 r_p = (eta * micronormal_dot_in - micronormal_dot_out) / (eta * micronormal_dot_in + micronormal_dot_out);

    return 0.5f * (POWF(r_s, 2) + POWF(r_p, 2));
}

intern inline f32 Fresnel_Achromatic(vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 eta)
{
    f32 micronormal_dot_in = vdot(w_micronormal, w_in);
    // f32 micronormal_dot_out = vdot(vmul(-1.0f, w_micronormal), w_out);
    f32 micronormal_dot_out = vdot(w_micronormal, w_out);

    return Fresnel_Achromatic_DotProducts(micronormal_dot_in, micronormal_dot_out, eta);
}

// see: https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
intern inline f32 Fresnel_Achromatic_IncidentOnly(f32 micronormal_dot_in, f32 eta)
{
    f32 micronormal_dot_out_2 = 1.0f - POWF(eta, 2) * (1.0f - POWF(micronormal_dot_in, 2));
    if (micronormal_dot_out_2 < 0.0f) {
        // total internal reflection
        return 1.0f;
    }

    f32 micronormal_dot_out = sqrtf(micronormal_dot_out_2);
    return Fresnel_Achromatic_DotProducts(micronormal_dot_in, micronormal_dot_out, eta);
}

intern inline f32 D_g(vec3 w_micronormal, f32 a_x, f32 a_y)
{
    return D_m(w_micronormal, a_x, a_y);
}

intern inline f32 G_g1(vec3 w, f32 a_x, f32 a_y)
{
    return G_m1(w, a_x, a_y);
}

intern inline f32 G_g2(vec3 w_in, vec3 w_out, vec3 w_micronormal, f32 a_x, f32 a_y)
{
    return G_m2(w_in, w_out, w_micronormal, a_x, a_y);
}

intern inline Color BRDF_Disney_Glass(
    Color base_color,
    vec3  w_in,
    vec3  w_out,
    vec3  w_micronormal,
    vec3  w_macronormal,
    f32   a_x,
    f32   a_y,
    f32   eta)
{
    if (vdot(w_macronormal, w_in) * vdot(w_macronormal, w_out) > 0.0f) {
        // Reflective case
        f32 f_g = Fresnel_Achromatic(w_in, w_out, w_micronormal, eta);
        f32 d_g = D_g(w_micronormal, a_x, a_y);
        f32 g_g = G_g2(w_in, w_out, w_micronormal, a_x, a_y);

        f32 numerator   = f_g * d_g * g_g;
        f32 denominator = 4.0f * fabsf(vdot(w_macronormal, w_in));

        return vmul(base_color, numerator / denominator);
    } else {
        // Refractive case
        Color sqrt_color = vec(sqrtf(base_color.r), sqrtf(base_color.g), sqrtf(base_color.b));
        f32   f_g        = Fresnel_Achromatic(w_in, w_out, w_micronormal, eta);
        f32   d_g        = D_g(w_micronormal, a_x, a_y);
        f32   g_g        = G_g2(w_in, w_out, w_micronormal, a_x, a_y);

        f32 micronormal_dot_out = vdot(w_micronormal, w_out);
        f32 micronormal_dot_in  = vdot(w_micronormal, w_in);

        f32 numerator   = (1.0f - f_g) * d_g * g_g * fabsf(micronormal_dot_out * micronormal_dot_in);
        f32 denominator = fabsf(vdot(w_macronormal, w_in)) * POWF(micronormal_dot_in + eta * micronormal_dot_out, 2);

        return vmul(sqrt_color, numerator / denominator);
    }
}

intern inline vec3 Disney_Glass_SampleNormal(vec3 w_in, f32 a_x, f32 a_y)
{
    // TODO: might need to clamp roughness to [0.01, 1.0] to avoid numerical precision issues
    f32  u0            = Random_Unilateral();
    f32  u1            = Random_Unilateral();
    vec3 w_micronormal = GGXVNDF_Sample(w_in, a_x, a_y, u0, u1);

    return w_micronormal;
}

// TODO: the handling of the macronormal/half vector vs micronormal seems suspicious to me
intern inline f32 Disney_Glass_SampleNormal_PDF(vec3 w_in, vec3 w_out, vec3 w_macronormal, f32 a_x, f32 a_y, f32 eta)
{
    // TODO: we know whether we reflected or not, maybe we should just pass that info in
    bool reflect = vdot(w_macronormal, w_in) * vdot(w_macronormal, w_out) > 0.0f;

    // Generalized half-vector in the refract case
    // See "Microfacet Models for Refraction through Rough Surfaces"
    vec3 w_micronormal = reflect ? HalfVector(w_in, w_out) : vnorm(vadd(w_in, vmul(eta, w_out)));

    // TODO: flip half_vector if below surface?
    // TODO: might need to clamp roughness to [0.01, 1.0] to avoid numerical precision issues

    f32 f_g  = Fresnel_Achromatic(w_in, w_out, w_micronormal, eta);
    f32 d_g  = D_g(w_micronormal, a_x, a_y);
    f32 g_g1 = G_g1(w_in, a_x, a_y);

    if (reflect) {
        // PDF =
        // F_g * D_g * G_g1(w_in)
        // ----------------------
        //   4.0f * |n . w_in|
        return (f_g * d_g * g_g1) / (4.0f * fabsf(vdot(w_macronormal, w_in)));
    } else {
#if 0
        f32 h_dot_in   = vdot(w_micronormal, w_in);
        f32 h_dot_out  = vdot(w_micronormal, w_out);
        f32 sqrt_denom = h_dot_in + eta * h_dot_out;
        f32 dh_dout    = eta * eta * h_dot_out / (sqrt_denom * sqrt_denom);
        // TODO: figure this out so we can do cancellation
        // PDF = ?
        return (1.0f - f_g) * d_g * g_g1 * fabsf(dh_dout * h_dot_in / vdot(w_macronormal, w_in));
#else
        f32 jacobian
            = vdot(w_micronormal, w_out) / POWF(eta * vdot(w_micronormal, w_in) + vdot(w_micronormal, w_out), 2);
        return (1.0f - f_g) * d_g * g_g1 * jacobian / vdot(w_macronormal, w_in);
#endif
    }
}

bool Material_Disney_Glass_Bounce(
    Material_Disney_Glass* mat,
    Ray*                   ray_in,
    HitInfo*               hit,
    Color*                 surface_color,
    Color*                 emitted_color,
    Ray*                   ray_out)
{
    // convert material parameters to a_x and a_y
    // TODO: no reason to do this on the fly, could be precomputed
    f32 aspect = sqrtf(1.0f - 0.9f * mat->anistropic);
    f32 a_min  = 0.0001f;
    f32 a_x    = maxf(a_min, POWF(mat->roughness, 2) / aspect);
    f32 a_y    = maxf(a_min, POWF(mat->roughness, 2) * aspect);

    // determine what eta should be (material param is internal/external)
    // TODO: this might be backwards
    f32 eta = hit->frontFace ? mat->eta : 1.0f / mat->eta;

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_basis = vec3_OrthonormalBasis(hit->unitNormal);
    normal_basis        = (basis3){.x = normal_basis.z, .y = normal_basis.y, .z = normal_basis.x};

    // TODO: if we make all rays have normalized direction we can remove the vnorm
    vec3 ray_in_dir = vnorm(vmul(-1.0f, ray_in->dir));
    vec3 w_in       = vec3_Reorient(ray_in_dir, normal_basis);

    vec3 w_macronormal = vec(0.0f, 0.0f, 1.0f);
    vec3 w_micronormal = Disney_Glass_SampleNormal(w_in, a_x, a_y);

    vec3 w_out;
    // compute w_out (Reflect vs Refract)
    // TODO: review these equations, there's probably some optimization opportunity here, also needs refactoring
    {
        // TODO: flip micronormal if below surface?
        f32 micronormal_dot_in = vdot(w_micronormal, w_in);
        f32 f_g                = Fresnel_Achromatic_IncidentOnly(micronormal_dot_in, eta);

        f32 u2 = Random_Unilateral();
        if (u2 <= f_g) {
            w_out = vnorm(vadd(vmul(-1.0f, w_in), vmul(2.0f * micronormal_dot_in, w_micronormal)));
        } else {
            // TODO: flip micronormal if below surface?
#if 1
            f32 micronormal_dot_out = sqrtf(1.0f - (1.0f - POWF(micronormal_dot_in, 2)) / POWF(eta, 2));
            w_out                   = vadd(
                vmul(-1.0f / eta, w_in),
                vmul((fabsf(micronormal_dot_in) / eta - micronormal_dot_out), w_micronormal));
#else
            w_out = vec3_Refract(vmul(-1.0f, w_in), w_micronormal, 1.0f / eta);
#endif
        }
    }

    w_out = vnorm(w_out);

    Color albedo = Texture_ColorAt(mat->albedo, hit->uv);

    Color brdf = BRDF_Disney_Glass(albedo, w_in, w_out, w_micronormal, w_macronormal, a_x, a_y, eta);
    f32   pdf  = Disney_Glass_SampleNormal_PDF(w_in, w_out, w_macronormal, a_x, a_y, eta);

    Color brdf_color = vdiv(brdf, pdf);

    // transform w_out into world space
    basis3 world_basis = (basis3){
        .x = vec(1.0f, 0.0f, 0.0f),
        .y = vec(0.0f, 1.0f, 0.0f),
        .z = vec(0.0f, 0.0f, 1.0f),
    };
    vec3 ray_out_dir = vec3_Reorient(w_out, world_basis);

    *surface_color = brdf_color;
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, ray_out_dir);

    return true;
}
