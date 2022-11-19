#pragma once

#include "math/vec.h"

vec3 Distribution_UniformHemisphere_Sample(vec3 unit_normal);
f32  Distribution_UniformHemisphere_PDF(void);

vec3 Distribution_CosWeightedHemisphere_Sample(vec3 unit_normal);
f32  Distribution_CosWeightedHemisphere_PDF(f32 theta);
f32  Distribution_CosWeightedHemisphere_PDF_InvNoCos(void);
