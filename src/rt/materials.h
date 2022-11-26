#pragma once

#include <stdbool.h>

#include "gfx/color.h"
#include "gfx/texture.h"
#include "math/vec.h"
#include "rt/ray.h"
#include "world/skybox.h"

// TODO: reference: https://cseweb.ucsd.edu/~tzli/cse272/homework1.pdf
// They use a Texture<Real>, I think this is basically a map of the parameters of the material over the surface
// so that you can have variability of material parameters on the object's surface
// This would be interesting to implement at some point, but it will be easier (and faster) to get the uniform
// parameters to work

typedef enum {
    /* raytracing.github.io Materials */
    MATERIAL_DIFFUSE,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
    MATERIAL_DIFFUSE_LIGHT,

    /* Custom Materials */
    MATERIAL_SKYBOX,

    /* Disney Materials */
    MATERIAL_DISNEY_DIFFUSE,
    MATERIAL_DISNEY_METAL,
    MATERIAL_DISNEY_GLASS,
    MATERIAL_DISNEY_CLEARCOAT,
    MATERIAL_DISNEY_SHEEN,
    MATERIAL_DISNEY_BSDF,
} Material_Type;

typedef struct {
    Texture* albedo;
} Material_Diffuse;

typedef struct {
    Texture* albedo;
    f32      fuzz;
} Material_Metal;

typedef struct {
    Texture* albedo;
    f32      refactiveIndex;
} Material_Dielectric;

typedef struct {
    Texture* albedo;
    f32      brightness;
} Material_DiffuseLight;

typedef struct {
    Skybox* skybox;
} Material_Skybox;

typedef struct {
    Texture* albedo;
    f32      subsurface;
    f32      specular;
    f32      roughness;
    f32      specular_tint;
    f32      anistropic;
    f32      sheen_tint;
    f32      clearcoat_gloss;
    f32      eta; // Internal_IOR / External_IOR

    struct {
        f32 clearcoat;
        f32 sheen;
        f32 metallic;
        f32 specular;
    } weights;
} Material_Disney_BSDF;

typedef struct {
    Material_Type type;

    union {
        Material_Diffuse      diffuse;
        Material_Metal        metal;
        Material_Dielectric   dielectric;
        Material_DiffuseLight diffuse_light;
        Material_Skybox       skybox;

        Material_Disney_BSDF disney;
    };
} Material;

Material Material_Diffuse_Make(Texture* tex);
Material Material_Metal_Make(Texture* tex, f32 fuzz);
Material Material_Dielectric_Make(Texture* tex, f32 refractiveIndex);
Material Material_DiffuseLight_Make(Texture* tex, f32 brightness);
Material Material_Skybox_Make(Skybox* skybox);

bool Material_Diffuse_Bounce(
    Material_Diffuse* lambert,
    Ray*              rayIn,
    HitInfo*          hit,
    Color*            colorSurface,
    Color*            colorEmitted,
    Ray*              rayOut);

bool Material_Metal_Bounce(
    Material_Metal* metal,
    Ray*            rayIn,
    HitInfo*        hit,
    Color*          colorSurface,
    Color*          colorEmitted,
    Ray*            rayOut);

bool Material_Dielectric_Bounce(
    Material_Dielectric* diel,
    Ray*                 rayIn,
    HitInfo*             hit,
    Color*               colorSurface,
    Color*               colorEmitted,
    Ray*                 rayOut);

bool Material_DiffuseLight_Bounce(
    Material_DiffuseLight* diffuseLight,
    Ray*                   rayIn,
    HitInfo*               hit,
    Color*                 colorSurface,
    Color*                 colorEmitted,
    Ray*                   rayOut);

bool Material_Skybox_Bounce(
    Material_Skybox* skybox,
    Ray*             rayIn,
    HitInfo*         hit,
    Color*           colorSurface,
    Color*           colorEmitted,
    Ray*             rayOut);

/* ---- Disney Materials ---- */

Material Material_Disney_Diffuse_Make(Texture* albedo, f32 roughness, f32 subsurface);
Material Material_Disney_Metal_Make(Texture* albedo, f32 roughness, f32 anistropic);
Material Material_Disney_Clearcoat_Make(f32 gloss);
Material Material_Disney_Glass_Make(Texture* albedo, f32 roughness, f32 anistropic, f32 eta);
Material Material_Disney_Sheen_Make(Texture* albedo, f32 sheen_tint);
Material Material_Disney_BSDF_Make(
    Texture* albedo,
    f32      subsurface,
    f32      specular,
    f32      roughness,
    f32      specular_tint,
    f32      anistropic,
    f32      sheen_tint,
    f32      clearcoat_gloss,
    f32      weight_sheen,
    f32      weight_clearcoat,
    f32      weight_specular,
    f32      weight_metallic);

bool Material_Disney_Diffuse_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);

bool Material_Disney_Metal_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);

bool Material_Disney_Clearcoat_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);

bool Material_Disney_Glass_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);

bool Material_Disney_Sheen_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);

bool Material_Disney_BSDF_Bounce(
    Material_Disney_BSDF* mat,
    Ray*                  ray_in,
    HitInfo*              hit,
    Color*                surface_color,
    Color*                emitted_color,
    Ray*                  ray_out);
