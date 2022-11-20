#pragma once

#include <assert.h>

#include "world/object.h"

typedef struct KDTree KDTree;

KDTree* KDTree_New(in Object* objs, size_t len);
void    KDTree_Delete(inout KDTree* tree);
bool    KDTree_HitAt(in KDTree* tree, in Ray* ray, out Object** objHit, out HitInfo* hit);
