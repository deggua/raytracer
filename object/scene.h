#pragma once

#include "object/kdtree.h"
#include "object/object.h"

#define REQ_PARAM_type Object
#include "common/vector.h"

typedef struct {
    Color skyColor;
    Vector(Object) * objects;
    Vector(Object) * unboundObjs;
    KDNode* root;
} Scene;

Scene* Scene_New(Color skyColor);
void   Scene_Delete(Scene* scene);
void   Scene_Prepare(Scene* scene);
bool   Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
// TODO:
bool Scene_AddObject(const Object* obj);
