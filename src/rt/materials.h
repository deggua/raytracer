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
    MATERIAL_TEST,
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
    const Texture* albedo;
    f32            param;
} Material_Test;

typedef struct {
    Material_Type type;

    union {
        Material_Diffuse      diffuse;
        Material_Metal        metal;
        Material_Dielectric   dielectric;
        Material_DiffuseLight diffuseLight;
        Material_Test         test;
    };
} Material;

Material Material_Diffuse_Make(const Texture* tex);
Material Material_Metal_Make(const Texture* tex, f32 fuzz);
Material Material_Dielectric_Make(const Texture* tex, f32 refractiveIndex);
Material Material_DiffuseLight_Make(const Texture* tex, f32 brightness);
Material Material_Test_Make(const Texture* tex, f32 param);

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

bool Material_Test_Bounce(
    const Material_Test* test,
    const Ray*           rayIn,
    const HitInfo*       hit,
    Color*               colorSurface,
    Color*               colorEmitted,
    Ray*                 rayOut);
