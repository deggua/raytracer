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
    f32      roughness;
    f32      subsurface;
} Material_Disney_Diffuse;

typedef struct {
    Texture* albedo;
    f32      roughness;
    f32      anistropic;
} Material_Disney_Metal;

typedef struct {
    Texture* albedo;
    f32      roughness;
    f32      anistropic;
    f32      eta; // Internal_IOR / External_IOR (looks like n in the formula)
} Material_Disney_Glass;

typedef struct {
    Texture* albedo;
    f32      gloss;
} Material_Disney_Clearcoat;

typedef struct {
    Texture* albedo;
    f32      sheen;
} Material_Disney_Sheen;

typedef struct {
    Texture* albedo;
    // TODO: finish this, seems complicated
} Material_Disney_BSDF;

typedef struct {
    Material_Type type;

    union {
        Material_Diffuse      diffuse;
        Material_Metal        metal;
        Material_Dielectric   dielectric;
        Material_DiffuseLight diffuse_light;
        Material_Skybox       skybox;

        Material_Disney_Diffuse   disney_diffuse;
        Material_Disney_Metal     disney_metal;
        Material_Disney_Glass     disney_glass;
        Material_Disney_Clearcoat disney_clearcoat;
        Material_Disney_Sheen     disney_sheen;
        Material_Disney_BSDF      disney_bsdf;
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

bool Material_Disney_Diffuse_Bounce(
    Material_Disney_Diffuse* mat,
    Ray*                     ray_in,
    HitInfo*                 hit,
    Color*                   surface_color,
    Color*                   emitted_color,
    Ray*                     ray_out);

bool Material_Disney_Metal_Bounce(
    Material_Disney_Metal* mat,
    Ray*                   ray_in,
    HitInfo*               hit,
    Color*                 surface_color,
    Color*                 emitted_color,
    Ray*                   ray_out);
