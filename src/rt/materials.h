#pragma once

#include <stdbool.h>

#include "common/common.h"
#include "common/vec.h"
#include "gfx/color.h"
#include "rt/ray.h"

typedef enum {
    MATERIAL_DIFFUSE,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
} MaterialType;

typedef struct {
    Color albedo;
} Diffuse;

typedef struct {
    Color albedo;
    f32   fuzz;
} Metal;

typedef struct {
    Color albedo;
    f32   refactiveIndex;
} Dielectric;

typedef struct {
    MaterialType type;
    union {
        Diffuse    diffuse;
        Metal      metal;
        Dielectric dielectric;
    };
} Material;

Diffuse Diffuse_Make(Color albedo);
bool    Diffuse_Bounce(const Diffuse* lambert, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);

Metal Metal_Make(Color albedo, f32 fuzz);
bool  Metal_Bounce(const Metal* metal, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);

Dielectric Dielectric_Make(Color albedo, f32 refractiveIndex);
bool       Dielectric_Bounce(const Dielectric* diel, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);
