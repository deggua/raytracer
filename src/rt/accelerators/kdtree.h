#pragma once

#include <assert.h>

#include "world/object.h"

typedef struct KDTree KDTree;

KDTree* KDTree_New(Object* objs, size_t len);
void    KDTree_Delete(KDTree* tree);
bool    KDTree_HitAt(KDTree* tree, Ray* ray, Object** objHit, HitInfo* hit);
