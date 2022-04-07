#pragma once

#include <assert.h>

#include "object/object.h"

typedef struct KDTree KDTree;

KDTree* KDTree_New(const Object objs[], size_t len, size_t numThreads);
void    KDTree_Delete(KDTree* tree);
bool    KDTree_HitAt(KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit, const size_t threadNum);
