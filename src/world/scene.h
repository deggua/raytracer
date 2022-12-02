#pragma once

#include "rt/accelerators/kdtree.h"
#include "world/object.h"
#include "world/skybox.h"

typedef struct Scene Scene;

Scene* Scene_New(Skybox* skybox);
void   Scene_Delete(Scene* scene);
bool   Scene_Prepare(Scene* scene);
bool   Scene_ClosestHit(Scene* scene, Ray* ray, Object** objHit, HitInfo* hit);

bool  Scene_Add_Object(Scene* scene, Object* obj);
Color Scene_Get_SkyColor(Scene* scene, vec3 dir);
