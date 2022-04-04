#include "scene.h"

#include <math.h>
#include <stdbool.h>

#include "common/cext.h"
#include "gfx/primitives.h"
#include "object/kdtree.h"
#include "object/object.h"

#define REQ_PARAM_type ObjectBB
#include "common/vector.h"

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

static bool KDSplitX(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth);
static bool KDSplitY(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth);
static bool KDSplitZ(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth);

static bool Scene_ClosestHitInKD(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth)
{
    switch (node->type) {
        case KD_PARENT: {
            if (kdDepth % 3 == 0) {
                return KDSplitX(node, ray, objHit, hit, kdDepth);
            } else if (kdDepth % 3 == 1) {
                return KDSplitY(node, ray, objHit, hit, kdDepth);
            } else if (kdDepth % 3 == 2) {
                return KDSplitZ(node, ray, objHit, hit, kdDepth);
            }
        } break;

        case KD_LEAF: {
            return Scene_ClosestHitInArray(node->objs.elem, node->objs.len, ray, objHit, hit);
        } break;
    };

    return false;
}

static bool KDSplitX(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth)
{
    const float split            = node->node.split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (fabsf(ray->dir.x) < epsilonParallel) {
        // ray parallel to plane
        if (ray->origin.x >= split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        }
    }

    float tIntersect = (split - ray->origin.x) / ray->dir.x;
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t
        if (ray->origin.x > split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else if (ray->origin.x < split) {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        } else {
            // handle ray intersects plane at the origin, idea is to evaluate at some point past the origin and see what
            // side that lands on
            // TODO: is this correct?
            float rayAt = Ray_At(ray, 1.0f).x;
            if (rayAt > split) {
                return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
            } else {
                return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite, since the ray will
        // hit objects on the origin side before objects on the opposite side
        KDNode* originSide;
        KDNode* oppositeSide;

        if (ray->origin.x >= split) {
            originSide   = node->node.gte;
            oppositeSide = node->node.lt;
        } else {
            originSide   = node->node.lt;
            oppositeSide = node->node.gte;
        }

        Object* tempObjHit;
        HitInfo tempHit;
        if (Scene_ClosestHitInKD(originSide, ray, &tempObjHit, &tempHit, kdDepth + 1)) {
            *objHit = tempObjHit;
            *hit    = tempHit;
            return true;
        } else {
            return Scene_ClosestHitInKD(oppositeSide, ray, objHit, hit, kdDepth + 1);
        }
    }
}

static bool KDSplitY(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth)
{
    const float split            = node->node.split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (fabsf(ray->dir.y) < epsilonParallel) {
        // ray parallel to plane
        if (ray->origin.y >= split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        }
    }

    float tIntersect = (split - ray->origin.y) / ray->dir.y;
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t
        if (ray->origin.y > split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else if (ray->origin.y < split) {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        } else {
            // handle ray intersects plane at the origin, idea is to evaluate at some point past the origin and see what
            // side that lands on
            // TODO: is this correct?
            float rayAt = Ray_At(ray, 1.0f).y;
            if (rayAt > split) {
                return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
            } else {
                return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite, since the ray will
        // hit objects on the origin side before objects on the opposite side
        KDNode* originSide;
        KDNode* oppositeSide;

        if (ray->origin.y >= split) {
            originSide   = node->node.gte;
            oppositeSide = node->node.lt;
        } else {
            originSide   = node->node.lt;
            oppositeSide = node->node.gte;
        }

        Object* tempObjHit;
        HitInfo tempHit;
        if (Scene_ClosestHitInKD(originSide, ray, &tempObjHit, &tempHit, kdDepth + 1)) {
            *objHit = tempObjHit;
            *hit    = tempHit;
            return true;
        } else {
            return Scene_ClosestHitInKD(oppositeSide, ray, objHit, hit, kdDepth + 1);
        }
    }
}

static bool KDSplitZ(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, size_t kdDepth)
{
    const float split            = node->node.split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (fabsf(ray->dir.z) < epsilonParallel) {
        // ray parallel to plane
        if (ray->origin.z >= split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        }
    }

    float tIntersect = (split - ray->origin.z) / ray->dir.z;
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t
        if (ray->origin.z > split) {
            return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
        } else if (ray->origin.z < split) {
            return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
        } else {
            // handle ray intersects plane at the origin, idea is to evaluate at some point past the origin and see what
            // side that lands on
            // TODO: is this correct?
            float rayAt = Ray_At(ray, 1.0f).z;
            if (rayAt > split) {
                return Scene_ClosestHitInKD(node->node.gte, ray, objHit, hit, kdDepth + 1);
            } else {
                return Scene_ClosestHitInKD(node->node.lt, ray, objHit, hit, kdDepth + 1);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite, since the ray will
        // hit objects on the origin side before objects on the opposite side
        KDNode* originSide;
        KDNode* oppositeSide;

        if (ray->origin.z >= split) {
            originSide   = node->node.gte;
            oppositeSide = node->node.lt;
        } else {
            originSide   = node->node.lt;
            oppositeSide = node->node.gte;
        }

        Object* tempObjHit;
        HitInfo tempHit;
        if (Scene_ClosestHitInKD(originSide, ray, &tempObjHit, &tempHit, kdDepth + 1)) {
            *objHit = tempObjHit;
            *hit    = tempHit;
            return true;
        } else {
            return Scene_ClosestHitInKD(oppositeSide, ray, objHit, hit, kdDepth + 1);
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
        Scene_ClosestHitInKD(scene->root, ray, &kdObjHit, &kdHit, 0);
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
