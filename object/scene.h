#pragma once

#include "object/kdtree.h"
#include "object/object.h"

typedef struct Scene Scene;

Scene* Scene_New(Color skyColor);
void   Scene_Delete(Scene* scene);
void   Scene_Prepare(Scene* scene);
bool   Scene_ClosestHit(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);
bool   Scene_ClosestHitSlow(const Scene* scene, const Ray* ray, Object** objHit, HitInfo* hit);

bool  Scene_Add_Object(Scene* scene, const Object* obj);
void  Scene_Set_SkyColor(Scene* scene, Color color);
Color Scene_Get_SkyColor(const Scene* scene);
