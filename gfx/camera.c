#include "camera.h"

#include <math.h>

#include "gfx/primitives.h"
#include "gfx/utils.h"

Camera* Camera_New(
    Point3 lookFrom,
    Point3 lookTo,
    Vec3   vup,
    float  aspectRatio,
    float  vertFov,
    float  aperature,
    float  focusDist,
    float  timeStart,
    float  timeEnd)
{
    const float theta          = radiansf(vertFov);
    const float hh             = tanf(theta / 2.0f);
    const float viewportHeight = 2.0f * hh;
    const float viewportWidth  = aspectRatio * viewportHeight;

    Camera* cam = calloc(1, sizeof(*cam));

    cam->w = vunit(vsub(lookFrom, lookTo));
    cam->u = vunit(vcross(vup, cam->w));
    cam->v = vcross(cam->w, cam->u);

    cam->origin     = lookFrom;
    cam->horizontal = vmul(cam->u, viewportWidth * focusDist);
    cam->vertical   = vmul(cam->v, viewportHeight * focusDist);
    cam->bottomLeftCorner
        = vsub(cam->origin, vmul(cam->horizontal, 0.5f), vmul(cam->vertical, 0.5f), vmul(cam->w, focusDist));

    cam->lensRadius = aperature / 2.0f;

    cam->timeStart = timeStart;
    cam->timeEnd   = timeEnd;

    return cam;
}

Ray Camera_GetRay(const Camera* cam, float u, float v)
{
    Vec3 rd     = vmul(Vec3_RandomInUnitDisc(), cam->lensRadius);
    Vec3 offset = (Vec3){
        .x = u * rd.x,
        .y = v * rd.y,
        .z = 0,
    };

    return (Ray){
        .origin = vadd(cam->origin, offset),
        .dir    = vsub(
            vadd(cam->bottomLeftCorner, vadd(vmul(cam->horizontal, u), vmul(cam->vertical, v))),
            vadd(cam->origin, offset)),
        .time = randrf(cam->timeStart, cam->timeEnd),
    };
}
