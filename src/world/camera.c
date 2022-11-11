#include "camera.h"

#include <math.h>
#include <stdlib.h>

#include "common/common.h"
#include "common/math.h"
#include "common/random.h"
#include "common/vec.h"

Camera* Camera_New(
    point3 lookFrom,
    point3 lookTo,
    vec3   vup,
    f32    aspectRatio,
    f32    vertFov,
    f32    aperature,
    f32    focusDist,
    f32    timeStart,
    f32    timeEnd)
{
    const f32 theta          = radiansf(vertFov);
    const f32 hh             = tanf(theta / 2.0f);
    const f32 viewportHeight = 2.0f * hh;
    const f32 viewportWidth  = aspectRatio * viewportHeight;

    Camera* cam = calloc(1, sizeof(*cam));

    cam->w = vnorm(vsub(lookFrom, lookTo));
    cam->u = vnorm(vcross(vup, cam->w));
    cam->v = vcross(cam->w, cam->u);

    cam->origin     = lookFrom;
    cam->horizontal = vmul(cam->u, viewportWidth * focusDist);
    cam->vertical   = vmul(cam->v, viewportHeight * focusDist);
    cam->bottomLeftCorner
        = vsub(cam->origin, vsum(vmul(cam->horizontal, 0.5f), vmul(cam->vertical, 0.5f), vmul(cam->w, focusDist)));

    cam->lensRadius = aperature / 2.0f;

    cam->timeStart = timeStart;
    cam->timeEnd   = timeEnd;

    return cam;
}

Ray Camera_GetRay(const Camera* cam, f32 u, f32 v)
{
    vec3 rd     = vmul(vec3_RandomInUnitDisc(), cam->lensRadius);
    vec3 offset = (vec3){
        .x = u * rd.x,
        .y = v * rd.y,
        .z = 0,
    };

    point3 rayOrigin = vadd(cam->origin, offset);
    vec3   rayDir    = vsub(
        vadd(cam->bottomLeftCorner, vadd(vmul(cam->horizontal, u), vmul(cam->vertical, v))),
        vadd(cam->origin, offset));

    return Ray_Make(rayOrigin, rayDir);
}
