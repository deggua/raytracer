#include "scene.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "math/math.h"
#include "math/vec.h"
#include "rt/accelerators/kdtree.h"
#include "world/object.h"
#include "world/skybox.h"

#define Vector_Type Object
#include "ctl/containers/vector.h"

typedef struct Scene {
    Skybox*         skybox;
    Vector(Object)* objects;
    Vector(Object)* unboundObjs;
    Vector(Object)* kdObjects;
    KDTree*         kdTree;
} Scene;

Scene* Scene_New(Skybox* skybox)
{
    Scene* scene = malloc(sizeof(Scene));
    if (scene == NULL) {
        goto error_Scene;
    }

    scene->objects = Vector_New(Object)(16);
    if (scene->objects == NULL) {
        goto error_ObjectVector;
    }

    scene->unboundObjs = Vector_New(Object)(16);
    if (scene->unboundObjs == NULL) {
        goto error_UnboundObjectsVector;
    }

    scene->kdObjects = Vector_New(Object)(16);
    if (scene->kdObjects == NULL) {
        goto error_KdObjects;
    }

    scene->skybox = skybox;

    return scene;

error_KdObjects:
    Vector_Delete(scene->unboundObjs);
error_UnboundObjectsVector:
    Vector_Delete(scene->objects);
error_ObjectVector:
    free(scene);
error_Scene:
    return NULL;
}

void Scene_Delete(Scene* scene)
{
    Vector_Delete(scene->objects);
    Vector_Delete(scene->unboundObjs);
    Vector_Delete(scene->kdObjects);

    if (scene->kdTree != NULL) {
        KDTree_Delete(scene->kdTree);
    }

    free(scene);
}

intern bool Scene_ClosestHitInArray(Object* objs, size_t len, Ray* ray, Object** objHit, HitInfo* hit)
{
    Object* closestObjectHit = NULL;
    HitInfo closestHit       = {.tIntersect = INF};

    for (size_t ii = 0; ii < len; ii++) {
        Object* obj = &objs[ii];

        HitInfo curHit;
        bool    hitDetected = Surface_HitAt(&obj->surface, ray, RT_EPSILON, INF, &curHit);

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

bool Scene_ClosestHit(Scene* scene, Ray* ray, Object** objHit, HitInfo* hit)
{
    return KDTree_HitAt(scene->kdTree, ray, objHit, hit);
}

bool Scene_ClosestHitSlow(Scene* scene, Ray* ray, Object** objHit, HitInfo* hit)
{
    return Scene_ClosestHitInArray(scene->objects->at, scene->objects->length, ray, objHit, hit);
}

bool Scene_Prepare(Scene* scene)
{
    // only bounded objects (not moving, not infinite) can be stored in the KDTree
    // TODO: can we store infinite objects? does it make sense to?
    Vector_Reserve(scene->kdObjects, scene->objects->length);
    printf("%zu primitives in scene\n", scene->objects->length);

    for (size_t ii = 0; ii < scene->objects->length; ii++) {
        // TODO: this is because I messed up the API between KDTree and Scene, I should add a Surface_Bounded() function
        // to check whether it is bounded or not
        BoundingBox ignore;

        if (Surface_BoundedBy(&scene->objects->at[ii].surface, &ignore)) {
            Vector_Push(scene->kdObjects, &scene->objects->at[ii]);
        } else {
            Vector_Push(scene->unboundObjs, &scene->objects->at[ii]);
        }
    }

    // now we need to construct the kd tree using the bounding boxes
    if (scene->kdObjects->length > 0) {
        scene->kdTree = KDTree_New(scene->kdObjects->at, scene->kdObjects->length);
    } else {
        scene->kdTree = NULL;
    }

    return true;
}

bool Scene_Add_Object(Scene* scene, Object* obj)
{
    return Vector_Push(scene->objects, obj);
}

Color Scene_Get_SkyColor(Scene* scene, vec3 dir)
{
    return Skybox_ColorAt(scene->skybox, dir);
}
