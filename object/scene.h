#pragma once

#include "object/object.h"

#define REQ_PARAM_type Object
#include "common/vector.h"

typedef struct {
    Color skyColor;
    Vector(Object)* objects;
} Scene;

Scene* Scene_New(Color skyColor);
void Scene_Delete(Scene* scene);
bool Scene_HitAny(const Scene* scene, const Ray* ray, Object** objHit);
bool Scene_HitAt(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
