#pragma once

#include "rt/accelerators/kdtree.h"
#include "world/object.h"
#include "world/skybox.h"

typedef struct Scene Scene;

Scene* Scene_New(Skybox* skybox);
void   Scene_Delete(Scene* scene);
bool   Scene_Prepare(Scene* scene);
bool   Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
bool   Scene_ClosestHitSlow(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);

bool  Scene_Add_Object(Scene* scene, const Object* obj);
Color Scene_Get_SkyColor(const Scene* scene, vec3 dir);
