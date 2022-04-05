#pragma once

#include <assert.h>

#include "object/object.h"

typedef enum
{
    KD_SPLIT_X = VEC_X,
    KD_SPLIT_Y = VEC_Y,
    KD_SPLIT_Z = VEC_Z,
    KD_LEAF,
} KDNodeType;

typedef struct KDNode KDNode;

typedef struct {
    size_t len;
    Object elem[];
} KDLeaf;

typedef struct {
    float   split;
    KDNode* lt;
    KDNode* gteq;
} KDParent;

typedef struct KDNode {
    KDNodeType type;
    union {
        KDParent node;
        KDLeaf   objs;
    };
} KDNode;

KDNode* KDTree_Build(ObjectBB* boxes, size_t len);
