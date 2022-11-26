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
    vec3 reflected = vec3_Reflect(rayIn->dir, hit->unitNormal);
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

    vec3 normRayDir = rayIn->dir;
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
        .disney = {
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

intern inline Color BRDF_Disney_BaseDiffuse(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
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

intern inline Color BRDF_Disney_Subsurface(Color base_color, vec3 w_in, vec3 w_out, vec3 shading_normal, f32 roughness)
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
    Color brdf_base_diffuse = BRDF_Disney_BaseDiffuse(base_color, w_in, w_out, shading_normal, roughness);
    Color brdf_subsurface   = BRDF_Disney_Subsurface(base_color, w_in, w_out, shading_normal, roughness);
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

    basis3 normal_to_world = vec3_OrthonormalBasis(unit_normal);
    vec3   xyz_world_space = vec3_Reorient(xyz_normal_space, normal_to_world);

    return xyz_world_space;
}

bool Material_Disney_Diffuse_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    // generate sample vector
    vec3 ray_dir_out = CosWeightedHemisphere_Sample(hit->unitNormal);

    vec3  shading_normal = hit->unitNormal;
    Color albedo         = Texture_ColorAt(mat->albedo, hit->uv);

    // w_in should point from the surface to the observer
    vec3 w_in  = vmul(-1.0f, ray_in->dir);
    vec3 w_out = ray_dir_out;

    // TODO: we can just pre-cancel the PI32 terms in the underlying BRDFs
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
        .disney = {
            .albedo           = albedo,
            .roughness        = roughness,
            .anistropic       = anistropic,
            .weights.metallic = 1.0f,
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

intern inline f32 G_m2(vec3 w_in, vec3 w_out, f32 a_x, f32 a_y)
{
    return G_m1(w_in, a_x, a_y) * G_m1(w_out, a_x, a_y);
}

intern inline f32 R_0(f32 eta)
{
    return POWF(eta - 1.0f, 2) / POWF(eta + 1.0f, 2);
}

intern inline Color C_0(Color base_color, Color k_s, f32 metallic, f32 specular, f32 eta)
{
    return vlerp(vmul(k_s, specular * R_0(eta)), base_color, metallic);
}

intern inline Color K_s(Color c_tint, f32 specular_tint)
{
    return vlerp(COLOR_WHITE, c_tint, specular_tint);
}

intern inline Color C_tint(Color base_color)
{
    f32 luminance = Color_Luminance(base_color);
    return vdiv(base_color, luminance);
}

intern inline Color BRDF_Disney_Metal(
    Color base_color,
    vec3  w_out,
    vec3  w_micronormal,
    f32   metallic,
    f32   specular,
    f32   specular_tint,
    f32   a_x,
    f32   a_y,
    f32   eta)
{
    Color k_s = K_s(C_tint(base_color), specular_tint);
    Color c_0 = C_0(base_color, k_s, metallic, specular, eta);
    Color f_m = Fresnel_Schlick_Chromatic(c_0, vdot(w_out, w_micronormal));
    f32   g_m = G_m1(w_out, a_x, a_y);

    return vmul(f_m, g_m);
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
    vec3 T1    = lensq > 0.0f ? vmul(vec(-Vh.y, Vh.x, 0.0f), (1.0f / sqrtf(lensq))) : vec(1.0f, 0.0f, 0.0f);
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
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    // convert material parameters to a_x and a_y
    // TODO: no reason to do this on the fly, could be precomputed
    f32 aspect = sqrtf(1.0f - 0.9f * mat->anistropic);
    f32 a_min  = 0.0001f;
    f32 a_x    = maxf(a_min, POWF(mat->roughness, 2) / aspect);
    f32 a_y    = maxf(a_min, POWF(mat->roughness, 2) * aspect);

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_to_world = vec3_OrthonormalBasis(hit->unitNormal);
    basis3 world_to_normal = vec3_OrthonormalBasis_Inverse(normal_to_world);

    vec3 ray_in_dir = vmul(-1.0f, ray_in->dir);
    vec3 w_in       = vec3_Reorient(ray_in_dir, world_to_normal);

    vec3 w_micronormal = Disney_Metal_SampleNormal(w_in, a_x, a_y);

    vec3 w_out = vnorm(vec3_Reflect(vmul(-1.0f, w_in), w_micronormal));

    Color albedo = Texture_ColorAt(mat->albedo, hit->uv);
    Color brdf   = BRDF_Disney_Metal(
        albedo,
        w_out,
        w_micronormal,
        mat->weights.metallic,
        mat->specular,
        mat->specular_tint,
        a_x,
        a_y,
        mat->eta);

    // transform w_out into world space
    vec3 ray_out_dir = vec3_Reorient(w_out, normal_to_world);

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
        .disney = {
            .clearcoat_gloss = gloss,
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

// TODO: should we just make this a wrapped call to Lambda_m?
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

intern inline f32 G_c2(vec3 w_in, vec3 w_out)
{
    return G_c1(w_in) * G_c1(w_out);
}

intern inline f32 BRDF_Disney_Clearcoat(vec3 w_in, vec3 w_out, vec3 w_micronormal, vec3 w_macronormal, f32 a_g)
{
    (void)a_g;
    f32 f_c = Fresnel_Schlick_Achromatic(w_out, w_micronormal);
    // f32 d_c = D_c(w_micronormal, a_g);
    f32 d_c = 1.0f; // due to term cancellation
    f32 g_c = G_c2(w_in, w_out);

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
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    // convert material param to a_g
    // TODO: no reason to do this on the fly, could be precomputed
    f32 a_g = (1.0f - mat->clearcoat_gloss) * 0.1f + mat->clearcoat_gloss * 0.001f;

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_to_world = vec3_OrthonormalBasis(hit->unitNormal);
    basis3 world_to_normal = vec3_OrthonormalBasis_Inverse(normal_to_world);

    vec3 ray_in_dir = vmul(-1.0f, ray_in->dir);
    vec3 w_in       = vec3_Reorient(ray_in_dir, world_to_normal);

    vec3 w_macronormal = vec(0.0f, 0.0f, 1.0f);
    vec3 w_micronormal = Disney_Clearcoat_SampleNormal(a_g);

    vec3 w_out = vnorm(vec3_Reflect(vmul(-1.0f, w_in), w_micronormal));

    f32 brdf    = BRDF_Disney_Clearcoat(w_in, w_out, w_micronormal, w_macronormal, a_g);
    f32 inv_pdf = Disney_Clearcoat_SampleNormal_InvPDF(w_out, w_micronormal, w_macronormal, a_g);

    Color brdf_color = vec(brdf * inv_pdf, brdf * inv_pdf, brdf * inv_pdf);

    // transform w_out into world space
    vec3 ray_out_dir = vec3_Reorient(w_out, normal_to_world);

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
        .disney = {
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
    f32 micronormal_dot_in  = vdot(w_micronormal, w_in);
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

intern inline f32 G_g2(vec3 w_in, vec3 w_out, f32 a_x, f32 a_y)
{
    return G_m2(w_in, w_out, a_x, a_y);
}

intern inline Color BRDF_Disney_Glass(Color base_color, vec3 w_out, f32 a_x, f32 a_y, bool reflected)
{
    if (reflected) {
        // Reflective case
        f32 g_g = G_g1(w_out, a_x, a_y);

        return vmul(base_color, g_g);
    } else {
        // Refractive case
        Color sqrt_color = vec(sqrtf(base_color.r), sqrtf(base_color.g), sqrtf(base_color.b));
        f32   g_g        = G_g1(w_out, a_x, a_y);

        return vmul(sqrt_color, g_g);
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

bool Material_Disney_Glass_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    // convert material parameters to a_x and a_y
    // TODO: no reason to do this on the fly, could be precomputed
    f32 aspect = sqrtf(1.0f - 0.9f * mat->anistropic);
    f32 a_min  = 0.0001f;
    f32 a_x    = maxf(a_min, POWF(mat->roughness, 2) / aspect);
    f32 a_y    = maxf(a_min, POWF(mat->roughness, 2) * aspect);

    // determine what eta should be (material param is internal/external)
    f32 eta = hit->frontFace ? 1.0f / mat->eta : mat->eta;

    // everything is done in a reference frame where the normal is <0, 0, 1> to make things simple
    basis3 normal_to_world = vec3_OrthonormalBasis(hit->unitNormal);
    basis3 world_to_normal = vec3_OrthonormalBasis_Inverse(normal_to_world);

    vec3 w_in = vec3_Reorient(vmul(-1.0f, ray_in->dir), world_to_normal);

    vec3 w_micronormal = Disney_Glass_SampleNormal(w_in, a_x, a_y);

    vec3 w_out;
    bool reflected;
    // compute w_out (Reflect vs Refract)
    // TODO: review these equations, there's probably some optimization opportunity here, also needs refactoring
    {
        f32 micronormal_dot_in = vdot(w_micronormal, w_in);
        f32 f_g                = Fresnel_Achromatic_IncidentOnly(micronormal_dot_in, eta);

        f32 u2 = Random_Unilateral();
        if (u2 <= f_g) {
            w_out     = vec3_Reflect(vmul(-1.0f, w_in), w_micronormal);
            reflected = true;
        } else {
            w_out     = vec3_Refract(vmul(-1.0f, w_in), w_micronormal, eta);
            reflected = false;
        }
    }

    // TODO: is this necessary?
    w_out = vnorm(w_out);

    Color albedo     = Texture_ColorAt(mat->albedo, hit->uv);
    Color brdf_color = BRDF_Disney_Glass(albedo, w_out, a_x, a_y, reflected);

    vec3 ray_out_dir = vec3_Reorient(w_out, normal_to_world);

    *surface_color = brdf_color;
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, ray_out_dir);

    return true;
}

/* ---- Disney Sheen ---- */

Material Material_Disney_Sheen_Make(Texture* albedo, f32 sheen_tint)
{
    return (Material){
        .type = MATERIAL_DISNEY_SHEEN,
        .disney = {
            .albedo = albedo,
            .sheen_tint = sheen_tint,
        },
    };
}

intern inline Color C_sheen(Color base_color, f32 sheen_tint)
{
    // return vadd(vmul(COLOR_WHITE, (1.0f - sheen_tint)), vmul(sheen_tint, C_tint(base_color)));
    return vlerp(COLOR_WHITE, C_tint(base_color), sheen_tint);
}

intern inline Color BRDF_Disney_Sheen(Color base_color, vec3 w_out, vec3 w_micronormal, f32 sheen_tint)
{
    return vmul(C_sheen(base_color, sheen_tint), POWF(1.0f - fabsf(vdot(w_micronormal, w_out)), 5));
}

bool Material_Disney_Sheen_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    vec3 w_in          = vmul(-1.0, ray_in->dir);
    vec3 w_out         = CosWeightedHemisphere_Sample(hit->unitNormal);
    vec3 w_micronormal = HalfVector(w_in, w_out);

    Color albedo     = Texture_ColorAt(mat->albedo, hit->uv);
    Color brdf_color = BRDF_Disney_Sheen(albedo, w_out, w_micronormal, mat->sheen_tint);
    f32   inv_pdf    = PI32;

    *surface_color = vmul(brdf_color, inv_pdf);
    *emitted_color = COLOR_BLACK;
    *ray_out       = Ray_Make(hit->position, w_out);

    return true;
}

/* ---- Disney BSDF ---- */

Material Material_Disney_BSDF_Make(
    Texture* albedo,
    f32      subsurface,
    f32      specular,
    f32      roughness,
    f32      specular_tint,
    f32      anistropic,
    f32      sheen_tint,
    f32      clearcoat_gloss,
    f32      eta,
    f32      weight_sheen,
    f32      weight_clearcoat,
    f32      weight_specular,
    f32      weight_metallic)
{
    return (Material){
        .type = MATERIAL_DISNEY_BSDF,
        .disney = {
            .albedo          = albedo,
            .subsurface      = subsurface,
            .specular        = specular,
            .roughness       = roughness,
            .specular_tint   = specular_tint,
            .anistropic      = anistropic,
            .sheen_tint      = sheen_tint,
            .clearcoat_gloss = clearcoat_gloss,
            .eta             = eta,

            .weights = {
                .clearcoat = weight_clearcoat,
                .sheen     = weight_sheen,
                .metallic  = weight_metallic,
                .specular  = weight_specular,
            },
        },
    };
}

bool Material_Disney_BSDF_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out)
{
    // TODO: this could be precomputed
    f32 weight_diffuse, weight_sheen, weight_metal, weight_clearcoat, weight_glass;
    if (hit->frontFace) {
        weight_diffuse   = (1.0f - mat->weights.specular) * (1.0f - mat->weights.metallic);
        weight_sheen     = (1.0f - mat->weights.metallic) * mat->weights.sheen;
        weight_metal     = (1.0f - mat->weights.specular * (1.0f - mat->weights.metallic));
        weight_clearcoat = 0.25f * mat->weights.clearcoat;
        weight_glass     = (1.0f - mat->weights.metallic) * mat->weights.specular;
    } else {
        weight_diffuse   = 0.0f;
        weight_sheen     = 0.0f;
        weight_metal     = 0.0f;
        weight_clearcoat = 0.0f;
        weight_glass     = (1.0f - mat->weights.metallic) * mat->weights.specular;
    }

    f32 total_weight = weight_diffuse + weight_sheen + weight_metal + weight_clearcoat + weight_glass;
    f32 sel_interval = Random_Unilateral() * total_weight;

    typedef struct {
        f32 lower, upper;
    } RngInterval;

    RngInterval diffuse   = {0.0f, weight_diffuse};
    RngInterval sheen     = {diffuse.upper, diffuse.upper + weight_sheen};
    RngInterval metal     = {sheen.upper, sheen.upper + weight_metal};
    RngInterval clearcoat = {metal.upper, metal.upper + weight_clearcoat};
    RngInterval glass     = {clearcoat.upper, clearcoat.upper + weight_glass};

    if (diffuse.lower <= sel_interval && sel_interval < diffuse.upper) {
        return Material_Disney_Diffuse_Bounce(mat, ray_in, hit, surface_color, emitted_color, ray_out);
    } else if (sheen.lower <= sel_interval && sel_interval < sheen.upper) {
        return Material_Disney_Sheen_Bounce(mat, ray_in, hit, surface_color, emitted_color, ray_out);
    } else if (metal.lower <= sel_interval && sel_interval < metal.upper) {
        // TODO: this needs the modified fresnel term, figure out how to incorporate
        return Material_Disney_Metal_Bounce(mat, ray_in, hit, surface_color, emitted_color, ray_out);
    } else if (clearcoat.lower <= sel_interval && sel_interval < clearcoat.upper) {
        return Material_Disney_Clearcoat_Bounce(mat, ray_in, hit, surface_color, emitted_color, ray_out);
    } else if (glass.lower <= sel_interval && sel_interval < glass.upper) {
        return Material_Disney_Glass_Bounce(mat, ray_in, hit, surface_color, emitted_color, ray_out);
    } else {
        // FIXME: This seems to happen because incident rays that are almost tangent are able to
        // produce reflected rays that are below the surface. This doesn't seem correct.
        // This case is only possible when specular_transmission is 0 and the ray is inside the object
        // which shouldn't be possible (as far as I can tell) but happens because of the above bug
        *surface_color = COLOR_BLACK;
        *emitted_color = COLOR_BLACK;
        return false;
    }
}
