#include "scene.h"

#include <math.h>
#include <stdbool.h>

#include "common/cext.h"
#include "gfx/primitives.h"
#include "object/kdtree.h"
#include "object/object.h"

#define REQ_PARAM_type Object
#include "common/vector.h"

#define REQ_PARAM_type ObjectBB
#include "common/vector.h"

typedef struct Scene {
    Color skyColor;
    Vector(Object) * objects;
    Vector(Object) * unboundObjs;
    KDNode* root;
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

    scene->skyColor = skyColor;
    return scene;

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

static bool CheckHitKD(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, VecAxis axis);

static bool Scene_ClosestHitInKD(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit)
{
    switch (node->type) {
        case KD_SPLIT_X:
        case KD_SPLIT_Y:
        case KD_SPLIT_Z: {
            return CheckHitKD(node, ray, objHit, hit, (VecAxis)(node->type));
        } break;

        case KD_LEAF: {
            return Scene_ClosestHitInArray(node->objs.elem, node->objs.len, ray, objHit, hit);
        } break;
    };

    // TODO: try to enable and see if it breaks code gen
    __builtin_unreachable();
    return false;
}

static bool CheckHitKD(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, VecAxis axis)
{
    const float split            = node->node.split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (fabsf(ray->dir.e[axis]) < epsilonParallel) {
        // ray parallel to plane, check the origin to see which side ray falls on
        // TODO: is this correct when the origin is in the plane?
        if (ray->origin.e[axis] >= split) {
            return Scene_ClosestHitInKD(node->node.gteq, ray, objHit, hit);
        } else {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit);
        }
    }

    float tIntersect = (split - ray->origin.e[axis]) / ray->dir.e[axis];
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t, check the origin to see which side ray falls on
        if (ray->origin.e[axis] > split) {
            return Scene_ClosestHitInKD(node->node.gteq, ray, objHit, hit);
        } else if (ray->origin.e[axis] < split) {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit);
        } else {
            // handle ray intersects plane at the origin, idea is to evaluate at some point past the origin and see what
            // side that lands on
            float rayAt = Ray_At(ray, 1.0f).e[axis];
            if (rayAt >= split) {
                return Scene_ClosestHitInKD(node->node.gteq, ray, objHit, hit);
            } else {
                return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side. we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite since the ray will
        // hit objects on the origin side before objects on the opposite side
        const KDNode* originSide;
        const KDNode* oppositeSide;

        if (ray->origin.e[axis] >= split) {
            originSide   = node->node.gteq;
            oppositeSide = node->node.lt;
        } else {
            originSide   = node->node.lt;
            oppositeSide = node->node.gteq;
        }

        if (Scene_ClosestHitInKD(originSide, ray, objHit, hit)) {
            return true;
        } else {
            return Scene_ClosestHitInKD(oppositeSide, ray, objHit, hit);
        }
    }
}

bool Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit)
{
    Object* ubObjHit = NULL;
    HitInfo ubHit;
    if (scene->unboundObjs->length > 0) {
        Scene_ClosestHitInArray(scene->unboundObjs->at, scene->unboundObjs->length, ray, &ubObjHit, &ubHit);
    }

    Object* kdObjHit = NULL;
    HitInfo kdHit;
    if (scene->root != NULL) {
        Scene_ClosestHitInKD(scene->root, ray, &kdObjHit, &kdHit);
    }

    if (kdObjHit == NULL && ubObjHit == NULL) {
        return false;
    } else if (kdObjHit != NULL && ubObjHit == NULL) {
        *objHit = kdObjHit;
        *hit    = kdHit;
    } else if (kdObjHit == NULL && ubObjHit != NULL) {
        *objHit = ubObjHit;
        *hit    = ubHit;
    } else {
        if (kdHit.tIntersect <= ubHit.tIntersect) {
            *objHit = kdObjHit;
            *hit    = kdHit;
        } else {
            *objHit = ubObjHit;
            *hit    = ubHit;
        }
    }

    return true;
}

bool Scene_ClosestHitSlow(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit)
{
    return Scene_ClosestHitInArray(scene->objects->at, scene->objects->length, ray, objHit, hit);
}

void Scene_Prepare(Scene* scene)
{
    // first we compute the bounding boxes for each object in our scene
    Vector(ObjectBB)* bObj = Vector_New(ObjectBB)();
    Vector_Reserve(ObjectBB)(bObj, scene->objects->length);

    for (size_t ii = 0; ii < scene->objects->length; ii++) {
        ObjectBB bb;
        if (Surface_BoundedBy(&scene->objects->at[ii].surface, &bb.box)) {
            bb.obj = &scene->objects->at[ii];
            Vector_Push(ObjectBB)(bObj, &bb);
        } else {
            Vector_Push(Object)(scene->unboundObjs, &scene->objects->at[ii]);
        }
    }

    // now we need to construct the kd tree using the bounding boxes
    if (bObj->length > 0) {
        scene->root = KDTree_Build(bObj->at, bObj->length);
    } else {
        scene->root = NULL;
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
