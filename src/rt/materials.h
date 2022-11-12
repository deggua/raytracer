#pragma once

#include <stdbool.h>

#include "common/vec.h"
#include "gfx/color.h"
#include "gfx/texture.h"
#include "rt/ray.h"

typedef enum {
    MATERIAL_DIFFUSE,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
    MATERIAL_DIFFUSE_LIGHT,
} MaterialType;

typedef struct {
    const Texture* albedo;
} Diffuse;

typedef struct {
    const Texture* albedo;
    f32            fuzz;
} Metal;

typedef struct {
    const Texture* albedo;
    f32            refactiveIndex;
} Dielectric;

typedef struct {
    const Texture* albedo;
    f32            brightness;
} DiffuseLight;

typedef struct {
    MaterialType type;

    union {
        Diffuse      diffuse;
        Metal        metal;
        Dielectric   dielectric;
        DiffuseLight diffuseLight;
    };
} Material;

Diffuse      Diffuse_Make(const Texture* tex);
Metal        Metal_Make(const Texture* tex, f32 fuzz);
Dielectric   Dielectric_Make(const Texture* tex, f32 refractiveIndex);
DiffuseLight DiffuseLight_Make(const Texture* tex, f32 brightness);

bool Diffuse_Bounce(
    const Diffuse* lambert,
    const Ray*     rayIn,
    const HitInfo* hit,
    Color*         colorSurface,
    Color*         colorEmitted,
    Ray*           rayOut);
bool Metal_Bounce(
    const Metal*   metal,
    const Ray*     rayIn,
    const HitInfo* hit,
    Color*         colorSurface,
    Color*         colorEmitted,
    Ray*           rayOut);
bool Dielectric_Bounce(
    const Dielectric* diel,
    const Ray*        rayIn,
    const HitInfo*    hit,
    Color*            colorSurface,
    Color*            colorEmitted,
    Ray*              rayOut);
bool DiffuseLight_Bounce(
    const DiffuseLight* diffuseLight,
    const Ray*          rayIn,
    const HitInfo*      hit,
    Color*              colorSurface,
    Color*              colorEmitted,
    Ray*                rayOut);
