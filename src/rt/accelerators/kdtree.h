#pragma once

#include <assert.h>

#include "world/object.h"

typedef struct KDTree KDTree;

KDTree* KDTree_New(const Object objs[], size_t len);
void    KDTree_Delete(KDTree* tree);
bool    KDTree_HitAt(const KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit);
