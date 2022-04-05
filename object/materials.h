#pragma once

#include <stdbool.h>

#include "gfx/color.h"
#include "gfx/primitives.h"
#include "object/hitinfo.h"

typedef enum
{
    MATERIAL_LAMBERT,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
} MaterialType;

typedef struct {
    Color albedo;
} Lambert;

typedef struct {
    Color albedo;
    float fuzz;
} Metal;

typedef struct {
    Color albedo;
    float refactiveIndex;
} Dielectric;

typedef struct {
    MaterialType type;
    union {
        Lambert    lambert;
        Metal      metal;
        Dielectric dielectric;
    };
} Material;

Lambert Lambert_Make(Color albedo);
bool    Lambert_Bounce(const Lambert* lambert, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);

Metal Metal_Make(Color albedo, float fuzz);
bool  Metal_Bounce(const Metal* metal, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);

Dielectric Dielectric_Make(Color albedo, float refractiveIndex);
bool       Dielectric_Bounce(const Dielectric* diel, const Ray* rayIn, const HitInfo* hit, Color* color, Ray* rayOut);
