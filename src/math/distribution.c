#include "distribution.h"

#include "math/math.h"
#include "math/random.h"

/* ---- Uniform Hemisphere ---- */

f32 Distribution_UniformHemisphere_PDF(void)
{
    return 1.0f / (2.0f * PI32);
}

// TODO: should we provide this? how should it take arguments?
f32 Distribution_UniformHemisphere_PDF_InvNoCos(f32 theta)
{
    // TODO: technically should be fabsf cos (I think)
    return 2.0f * PI32 * cosf(theta);
}

vec3 Distribution_UniformHemisphere_Sample(vec3 facing)
{
    return Random_InHemisphere(facing, 1.0f);
}

/* ---- Cosine Weighted Hemisphere ---- */

f32 Distribution_CosWeightedHemisphere_PDF(f32 theta)
{
    return cosf(theta) / PI32;
}

f32 Distribution_CosWeightedHemisphere_PDF_InvNoCos(void)
{
    return PI32;
}

vec3 Distribution_CosWeightedHemisphere_Sample(vec3 unit_normal)
{
    // TODO: convert to the dot product version, probably cheaper to compute

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
