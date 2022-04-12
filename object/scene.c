#include "scene.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "common/cext.h"
#include "gfx/primitives.h"
#include "object/kdtree.h"
#include "object/object.h"

#define REQ_PARAM_type Object
#include "common/vector.h"

typedef struct Scene {
    Color skyColor;
    Vector(Object) * objects;
    Vector(Object) * unboundObjs;
    Vector(Object) * kdObjects;
    KDTree* kdTree;
} Scene;

Scene* Scene_New(Color skyColor)
{
    Scene* scene = malloc(sizeof(Scene));
    if (scene == NULL) {
        goto error_Scene;
    }

    scene->objects = Vector_New(Object)();
    if (scene->objects == NULL) {
        goto error_ObjectVector;
    }

    scene->unboundObjs = Vector_New(Object)();
    if (scene->unboundObjs == NULL) {
        goto error_UnboundObjectsVector;
    }

    scene->kdObjects = Vector_New(Object)();
    if (scene->kdObjects == NULL) {
        goto error_KdObjects;
    }

    scene->skyColor = skyColor;
    return scene;

error_KdObjects:
    Vector_Delete(Object)(scene->unboundObjs);
error_UnboundObjectsVector:
    Vector_Delete(Object)(scene->objects);
error_ObjectVector:
    free(scene);
error_Scene:
    return NULL;
}

void Scene_Delete(Scene* scene)
{
    Vector_Delete(Object)(scene->objects);
    free(scene);
}

static bool Scene_ClosestHitInArray(const Object objs[], size_t len, const Ray* ray, Object** objHit, HitInfo* hit)
{
    const Object* closestObjectHit = NULL;
    HitInfo       closestHit       = {.tIntersect = INF};

    for (size_t ii = 0; ii < len; ii++) {
        const Object* obj = &objs[ii];

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
        *objHit = (Object*)closestObjectHit;
        return true;
    } else {
        return false;
    }
}

bool Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit)
{
    return KDTree_HitAt(scene->kdTree, ray, objHit, hit);
}

bool Scene_ClosestHitSlow(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit)
{
    return Scene_ClosestHitInArray(scene->objects->at, scene->objects->length, ray, objHit, hit);
}

void Scene_Prepare(Scene* scene)
{
    // only bounded objects (not moving, not infinite) can be stored in the KDTree
    // TODO: can we store infinite objects? does it make sense to?
    Vector_Reserve(Object)(scene->kdObjects, scene->objects->length);

    for (size_t ii = 0; ii < scene->objects->length; ii++) {
        // TODO: this is because I messed up the API between KDTree and Scene, I should add a Surface_Bounded() function
        // to check whether it is bounded or not
        BoundingBox ignore;
        if (Surface_BoundedBy(&scene->objects->at[ii].surface, &ignore)) {
            Vector_Push(Object)(scene->kdObjects, &scene->objects->at[ii]);
        } else {
            Vector_Push(Object)(scene->unboundObjs, &scene->objects->at[ii]);
            printf("scene->unboundObjs->length = %zu\n", scene->unboundObjs->length);
        }
    }

    // now we need to construct the kd tree using the bounding boxes
    if (scene->kdObjects->length > 0) {
        scene->kdTree = KDTree_New(scene->kdObjects->at, scene->kdObjects->length);
    } else {
        scene->kdTree = NULL;
    }
}

bool Scene_Add_Object(Scene* scene, const Object* obj)
{
    return Vector_Push(Object)(scene->objects, obj);
}

void Scene_Set_SkyColor(Scene* scene, Color color)
{
    scene->skyColor = color;
}

Color Scene_Get_SkyColor(const Scene* scene)
{
    return scene->skyColor;
}
