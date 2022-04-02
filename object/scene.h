#pragma once

#include "object/object.h"

#define REQ_PARAM_type Object
#include "common/vector.h"

typedef struct {
    Vector(Object)* objects;
} Scene;

Scene* Scene_New(void);
void Scene_Delete(Scene* scene);
bool Scene_HitAny(Scene* scene, const Ray* ray, Object** objHit);
bool Scene_HitAt(Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
