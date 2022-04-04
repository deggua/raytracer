#pragma once

#include <assert.h>

#include "object/object.h"

typedef enum
{
    KD_LEAF,
    KD_PARENT,
} KDNodeType;

typedef struct KDNode KDNode;

typedef struct {
    size_t len;
    Object elem[];
} KDLeaf;

typedef struct {
    float   split;
    KDNode* lt;
    KDNode* gte;
} KDParent;

typedef struct KDNode {
    KDNodeType type;
    union {
        KDParent node;
        KDLeaf   objs;
    };
} KDNode;

KDNode* KDTree_Build(ObjectBB* boxes, size_t len);
