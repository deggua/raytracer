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
    /* Raytracing.io Materials */
    MATERIAL_DIFFUSE,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
    MATERIAL_DIFFUSE_LIGHT,
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
    const Texture* albedo;
} Material_Diffuse;

typedef struct {
    const Texture* albedo;
    f32            fuzz;
} Material_Metal;

typedef struct {
    const Texture* albedo;
    f32            refactiveIndex;
} Material_Dielectric;

typedef struct {
    const Texture* albedo;
    f32            brightness;
} Material_DiffuseLight;

typedef struct {
    const Skybox* skybox;
} Material_Skybox;

typedef struct {
    const Texture* albedo;
    f32            roughness;
    f32            subsurface;
} Material_Disney_Diffuse;

typedef struct {
    const Texture* albedo;
    f32            roughness;
    f32            anistropic;
} Material_Disney_Metal;

typedef struct {
    const Texture* albedo;
    f32            roughness;
    f32            anistropic;
    f32            eta; // Internal_IOR / External_IOR (looks like n in the formula)
} Material_Disney_Glass;

typedef struct {
    const Texture* albedo;
    f32            gloss;
} Material_Disney_Clearcoat;

typedef struct {
    const Texture* albedo;
    f32            sheen;
} Material_Disney_Sheen;

typedef struct {
    const Texture* albedo;
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

Material Material_Diffuse_Make(const Texture* tex);
Material Material_Metal_Make(const Texture* tex, f32 fuzz);
Material Material_Dielectric_Make(const Texture* tex, f32 refractiveIndex);
Material Material_DiffuseLight_Make(const Texture* tex, f32 brightness);
Material Material_Skybox_Make(const Skybox* skybox);

Material Material_Disney_Diffuse_Make(const Texture* albedo, f32 roughness, f32 subsurface);

bool Material_Diffuse_Bounce(
    const Material_Diffuse* lambert,
    const Ray*              rayIn,
    const HitInfo*          hit,
    Color*                  colorSurface,
    Color*                  colorEmitted,
    Ray*                    rayOut);

bool Material_Metal_Bounce(
    const Material_Metal* metal,
    const Ray*            rayIn,
    const HitInfo*        hit,
    Color*                colorSurface,
    Color*                colorEmitted,
    Ray*                  rayOut);

bool Material_Dielectric_Bounce(
    const Material_Dielectric* diel,
    const Ray*                 rayIn,
    const HitInfo*             hit,
    Color*                     colorSurface,
    Color*                     colorEmitted,
    Ray*                       rayOut);

bool Material_DiffuseLight_Bounce(
    const Material_DiffuseLight* diffuseLight,
    const Ray*                   rayIn,
    const HitInfo*               hit,
    Color*                       colorSurface,
    Color*                       colorEmitted,
    Ray*                         rayOut);

bool Material_Skybox_Bounce(
    const Material_Skybox* skybox,
    const Ray*             rayIn,
    const HitInfo*         hit,
    Color*                 colorSurface,
    Color*                 colorEmitted,
    Ray*                   rayOut);

/* ---- Disney Materials ---- */

bool Material_Disney_Diffuse_Bounce(
    const Material_Disney_Diffuse* mat,
    const Ray*                     ray_in,
    const HitInfo*                 hit,
    Color*                         surface_color,
    Color*                         emitted_color,
    Ray*                           ray_out);
