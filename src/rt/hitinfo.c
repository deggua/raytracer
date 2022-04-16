#include "hitinfo.h"

void HitInfo_SetFaceNormal(HitInfo* hit, const Ray* ray, vec3 outwardNormal)
{
    hit->frontFace  = vdot(ray->dir, outwardNormal) < 0;
    hit->unitNormal = hit->frontFace ? outwardNormal : vmul(outwardNormal, -1.0f);
}
