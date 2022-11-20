#pragma once

#include "rt/accelerators/kdtree.h"
#include "world/object.h"
#include "world/skybox.h"

typedef struct Scene Scene;

Scene* Scene_New(in Skybox* skybox);
void   Scene_Delete(inout Scene* scene);
bool   Scene_Prepare(inout Scene* scene);
bool   Scene_ClosestHit(in Scene* scene, in Ray* ray, out Object** objHit, out HitInfo* hit);
bool   Scene_ClosestHitSlow(in Scene* scene, in Ray* ray, out Object** objHit, out HitInfo* hit);

bool  Scene_Add_Object(inout Scene* scene, in Object* obj);
Color Scene_Get_SkyColor(in Scene* scene, vec3 dir);
