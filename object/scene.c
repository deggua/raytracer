#include "scene.h"

#include <stdbool.h>

#include "common/cext.h"
#include "gfx/primitives.h"
#include "object/object.h"

Scene* Scene_New(void)
{
    Scene* scene   = malloc(sizeof(Scene));
    scene->objects = Vector_New(Object)();
    return scene;
}

void Scene_Delete(Scene* scene)
{
    Vector_Delete(Object)(scene->objects);
    free(scene);
}

bool Scene_HitAny(Scene* scene, const Ray* ray, Object** objHit)
{
    HitInfo ignore;
    for (size_t ii = 0; ii < scene->objects->length; ii++) {
        Object* obj = &scene->objects->at[ii];
        if (Surface_HitAt(&obj->surface, ray, 0.001f, INF, &ignore)) {
            *objHit = obj;
            return true;
        }
    }

    *objHit = NULL;
    return false;
}

bool Scene_HitAt(Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit)
{
    Object* closestObjectHit = NULL;
    HitInfo closestHit       = {.tIntersect = INF};

    for (size_t ii = 0; ii < scene->objects->length; ii++) {
        Object* obj = &scene->objects->at[ii];

        HitInfo curHit;
        bool    hitDetected = Surface_HitAt(&obj->surface, ray, 0.001f, INF, &curHit);
        if (hitDetected && (curHit.tIntersect < closestHit.tIntersect)) {
            // store the nearest intersection if the ray hits multiple objects
            closestHit       = curHit;
            closestObjectHit = obj;
        }
    }

    if (closestObjectHit != NULL) {
        *hit    = closestHit;
        *objHit = closestObjectHit;
        return true;
    } else {
        return false;
    }
}
