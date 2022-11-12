#pragma once

#include "rt/accelerators/kdtree.h"
#include "world/object.h"

typedef struct Scene Scene;

Scene* Scene_New(void);
void   Scene_Delete(Scene* scene);
bool   Scene_Prepare(Scene* scene);
bool   Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
bool   Scene_ClosestHitSlow(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);

bool  Scene_Add_Object(Scene* scene, const Object* obj);
void  Scene_Set_SkyColor(Scene* scene, Color color);
Color Scene_Get_SkyColor(const Scene* scene);
