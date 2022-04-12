#include "kdtree.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/cext.h"
#include "common/memory_arena.h"
#include "gfx/utils.h"
#include "object/object.h"

#define LEAF_MIN_LOAD 1
#define NUM_THREADS   4

typedef enum
{
    KD_INTERNAL_X = VEC_X,
    KD_INTERNAL_Y = VEC_Y,
    KD_INTERNAL_Z = VEC_Z,
    KD_LEAF,
} KDNodeType;

typedef struct {
    BoundingBox   box;
    const Object* obj;
} KDBB;

typedef struct KDNode KDNode;

typedef struct {
    size_t         len;
    const Object** objs;
} KDLeaf;

typedef struct {
    KDNode* left;
    KDNode* right;
    float   split;
} KDInternal;

typedef struct KDNode {
    KDNodeType type;
    union {
        KDInternal inode;
        KDLeaf     leaf;
    };
} KDNode;

typedef struct KDTree {
    KDNode*      root;
    MemoryArena* nodeArena;
    MemoryArena* objArena;
} KDTree;

const float TraversalCost = 1.0f;
const float IntersectCost = 3.0f;

static KDNode* BuildNode(KDTree* tree, const KDBB** kdbbs, size_t len, BoundingBox container);

static KDNode* BuildParentNode(
    KDTree*      tree,
    const KDBB** leftHalf,
    size_t       leftLen,
    BoundingBox  leftContainer,
    const KDBB** rightHalf,
    size_t       rightLen,
    BoundingBox  rightContainer,
    float        partition,
    VecAxis      axis)
{
    KDNode* leftNode = BuildNode(tree, leftHalf, leftLen, leftContainer);
    if (leftNode == NULL) {
        goto error_lt;
    }

    KDNode* parent = MemoryArena_Malloc(tree->nodeArena, sizeof(*parent));
    if (parent == NULL) {
        goto error_parent;
    }

    KDNode* rightNode = BuildNode(tree, rightHalf, rightLen, rightContainer);
    if (rightNode == NULL) {
        goto error_gteq;
    }

    parent->type        = (KDNodeType)axis;
    parent->inode.left  = leftNode;
    parent->inode.right = rightNode;
    parent->inode.split = partition;

    return parent;

error_parent:
error_gteq:
error_lt:
    return NULL;
}

static KDNode* BuildLeafNode(KDTree* tree, const KDBB** objs, size_t len)
{
    KDNode* node = MemoryArena_Malloc(tree->nodeArena, sizeof(*node));
    if (node == NULL) {
        return NULL;
    }

    // initialize the node and copy in the KDMetadata pointers
    node->type     = KD_LEAF;
    node->leaf.len = len;

    const Object** leafObjs = MemoryArena_Malloc(tree->objArena, len * sizeof(Object*));
    for (size_t ii = 0; ii < len; ii++) {
        leafObjs[ii] = objs[ii]->obj;
    }

    node->leaf.objs = leafObjs;

    return node;
}

static float SurfaceArea(BoundingBox box)
{
    const float xDim = box.max.x - box.min.x;
    const float yDim = box.max.y - box.min.y;
    const float zDim = box.max.z - box.min.z;

    return 2 * xDim * yDim + 2 * xDim * zDim + 2 * yDim * zDim;
}

static BoundingBox BoxBoundingAll(const KDBB** kdbbs, size_t len)
{
    BoundingBox box = {
        .min = kdbbs[0]->box.min,
        .max = kdbbs[0]->box.max,
    };

    for (size_t ii = 1; ii < len; ii++) {
        for (size_t axis = 0; axis < 3; axis++) {
            if (kdbbs[ii]->box.min.e[axis] < box.min.e[axis]) {
                box.min.e[axis] = kdbbs[ii]->box.min.e[axis];
            }

            if (kdbbs[ii]->box.max.e[axis] > box.max.e[axis]) {
                box.max.e[axis] = kdbbs[ii]->box.max.e[axis];
            }
        }
    }

    return box;
}

typedef struct {
    BoundingBox left;
    BoundingBox right;
} BoundingBoxPair;

static BoundingBoxPair SplitBox(BoundingBox box, float split, VecAxis axis)
{
    BoundingBoxPair splitBoxes = {
        .left  = box,
        .right = box,
    };
    splitBoxes.left.max.e[axis]  = split;
    splitBoxes.right.min.e[axis] = split;

    return splitBoxes;
}

static float ComputeSplitSAH(const KDBB** kdbbs, size_t len, float split, VecAxis axis, BoundingBox parent)
{
    const float parentSA = SurfaceArea(parent);

    BoundingBoxPair boxPair    = SplitBox(parent, split, axis);
    size_t          leftPrims  = 0;
    size_t          rightPrims = 0;

    for (size_t ii = 0; ii < len; ii++) {
        const BoundingBox* box = &kdbbs[ii]->box;
        if (box->max.e[axis] < split) {
            leftPrims += 1;
        } else if (box->min.e[axis] > split) {
            rightPrims += 1;
        } else {
            leftPrims += 1;
            rightPrims += 1;
        }
    }

    return TraversalCost + (SurfaceArea(boxPair.left) / parentSA) * leftPrims * IntersectCost
           + (SurfaceArea(boxPair.right) / parentSA) * rightPrims * IntersectCost;
}

static KDNode* BuildNode(KDTree* tree, const KDBB** kdbbs, size_t len, BoundingBox container)
{
    if (len <= LEAF_MIN_LOAD) {
        return BuildLeafNode(tree, kdbbs, len);
    }

    float   bestSplit = 0.0f;
    VecAxis bestAxis  = VEC_X;
    float   bestSAH   = INF;

    // determine best split location via SAH
    for (VecAxis axis = VEC_X; axis < VEC_LAST; axis++) {
        for (size_t ii = 0; ii < len; ii++) {
            for (size_t jj = 0; jj < 2; jj++) {
                float split = kdbbs[ii]->box.bounds[jj].e[axis];
                float SAH   = ComputeSplitSAH(kdbbs, len, split, axis, container);
                if (SAH < bestSAH) {
                    bestSAH   = SAH;
                    bestAxis  = axis;
                    bestSplit = split;
                }
            }
        }
    }

    float parentSAH = len * IntersectCost;
    if (parentSAH <= bestSAH) {
        // cost of best split outweighs just putting everything in a leaf
        return BuildLeafNode(tree, kdbbs, len);
    } else {
        // cost of best split is better than cost of total, split them up
        const KDBB** leftKdbb = calloc(len, sizeof(KDBB*));
        if (leftKdbb == NULL) {
            return NULL;
        }

        const KDBB** rightKdbb = calloc(len, sizeof(KDBB*));
        if (rightKdbb == NULL) {
            return NULL;
        }

        size_t leftLen  = 0;
        size_t rightLen = 0;

        for (size_t ii = 0; ii < len; ii++) {
            const BoundingBox* box = &kdbbs[ii]->box;
            if (box->max.e[bestAxis] < bestSplit) {
                leftKdbb[leftLen] = kdbbs[ii];
                leftLen += 1;
            } else if (box->min.e[bestAxis] > bestSplit) {
                rightKdbb[rightLen] = kdbbs[ii];
                rightLen += 1;
            } else {
                leftKdbb[leftLen] = kdbbs[ii];
                leftLen += 1;
                rightKdbb[rightLen] = kdbbs[ii];
                rightLen += 1;
            }
        }

        BoundingBoxPair pair = SplitBox(container, bestSplit, bestAxis);
        KDNode*         node
            = BuildParentNode(tree, leftKdbb, leftLen, pair.left, rightKdbb, rightLen, pair.right, bestSplit, bestAxis);

        free(leftKdbb);
        free(rightKdbb);

        return node;
    }

    return NULL;
}

static KDNode* BuildKDTree(KDTree* tree, const KDBB* boxes, size_t len)
{
    // we need a buffer of pointers to the objects BB's
    const KDBB** pBoxes = calloc(len, sizeof(KDBB*));
    if (pBoxes == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        pBoxes[ii] = &boxes[ii];
    }

    BoundingBox container = BoxBoundingAll(pBoxes, len);
    return BuildNode(tree, pBoxes, len, container);
}

static KDBB* CreateKDBBs(const Object objs[], size_t len)
{
    KDBB* boxes = calloc(len, sizeof(KDBB));
    if (boxes == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        boxes[ii].obj = &objs[ii];
        Surface_BoundedBy(&objs[ii].surface, &boxes[ii].box);
    }

    return boxes;
}

KDTree* KDTree_New(const Object objs[], size_t len)
{
    KDTree* tree = calloc(1, sizeof(KDTree));
    if (tree == NULL) {
        goto error_KDTree;
    }

    // max number of inodes = len - 1
    // max number of leaf nodes = len
    const size_t alignment = 8;
    const size_t nodeMem   = (2 * len - 1) * alignto(sizeof(KDNode), alignment - 1);
    const size_t totalMem  = alignto(nodeMem, alignment - 1);
    MemoryArena* nodeArena = MemoryArena_New(4 * totalMem, alignment);
    if (nodeArena == NULL) {
        goto error_NodeArena;
    }
    tree->nodeArena = nodeArena;

    // max number of object pointers allocated = objects * leaf nodes
    const size_t objMem   = len * len * alignto(sizeof(Object*), alignment - 1);
    MemoryArena* objArena = MemoryArena_New(4 * objMem, alignment);
    if (objArena == NULL) {
        goto error_ObjArena;
    }
    tree->objArena = objArena;

    KDBB* boxes = CreateKDBBs(objs, len);
    if (boxes == NULL) {
        goto error_KDBBs;
    }

    tree->root = BuildKDTree(tree, boxes, len);
    if (tree->root == NULL) {
        goto error_Root;
    }
    free(boxes);

    return tree;

error_Root:
    free(boxes);
error_KDBBs:
    MemoryArena_Delete(objArena);
error_ObjArena:
    MemoryArena_Delete(nodeArena);
error_NodeArena:
    free(tree);
error_KDTree:
    return NULL;
}

void KDTree_Delete(KDTree* tree)
{
    MemoryArena_Delete(tree->nodeArena);
    MemoryArena_Delete(tree->objArena);
    free(tree);
}

static bool CheckHitLeafNode(const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit);

static bool CheckHitInternalNode(const KDInternal* node, const Ray* ray, Object** objHit, HitInfo* hit, VecAxis axis);

static bool CheckHitNextNode(const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit)
{
    switch (node->type) {
        case KD_INTERNAL_X:
        case KD_INTERNAL_Y:
        case KD_INTERNAL_Z: {
            return CheckHitInternalNode(&node->inode, ray, objHit, hit, (VecAxis)node->type);
        }

        case KD_LEAF: {
            return CheckHitLeafNode(&node->leaf, ray, objHit, hit);
        } break;
    };

    return false;
}

static bool CheckHitLeafNode(const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit)
{
    HitInfo       hitClosest = {.tIntersect = INF};
    const Object* objClosest = NULL;

    for (size_t ii = 0; ii < leaf->len; ii++) {
        HitInfo hitCur;

        if (Surface_HitAt(&leaf->objs[ii]->surface, ray, 0.001f, INF, &hitCur)) {
            if (hitCur.tIntersect < hitClosest.tIntersect) {
                objClosest = leaf->objs[ii];
                hitClosest = hitCur;
            }
        }
    }

    if (objClosest != NULL) {
        *hit    = hitClosest;
        *objHit = (Object*)objClosest;
        return true;
    }

    return false;
}

static bool CheckHitInternalNode(const KDInternal* node, const Ray* ray, Object** objHit, HitInfo* hit, VecAxis axis)
{
    const float split            = node->split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (unlikely(fabsf(ray->dir.e[axis]) < epsilonParallel)) {
        // ray parallel to plane, check the origin to see which side ray falls on
        // TODO: is this correct when the origin is in the plane?
        if (ray->origin.e[axis] >= split) {
            return CheckHitNextNode(node->right, ray, objHit, hit);
        } else {
            return CheckHitNextNode(node->left, ray, objHit, hit);
        }
    }

    float tIntersect = split * ray->cache.invDir.e[axis] - ray->cache.originDivDir.e[axis];
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t, check the origin to see which side ray falls on
        if (likely(ray->origin.e[axis] > split)) {
            return CheckHitNextNode(node->right, ray, objHit, hit);
        } else if (likely(ray->origin.e[axis] < split)) {
            return CheckHitNextNode(node->left, ray, objHit, hit);
        } else {
            // ray intersects the plane at the origin, eval the ray later and see what side it's on
            // TODO: couldn't we just look at the direction? or just compute the sum of the origin + direction?
            // OPTIMIZE
            float rayAt = Ray_At(ray, 1.0f).e[axis];
            if (rayAt >= split) {
                return CheckHitNextNode(node->right, ray, objHit, hit);
            } else {
                return CheckHitNextNode(node->left, ray, objHit, hit);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side. we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite since the ray will
        // hit objects on the origin side before objects on the opposite side
        const KDNode* originSide;
        const KDNode* oppositeSide;

        if (ray->origin.e[axis] >= split) {
            originSide   = node->right;
            oppositeSide = node->left;
            // if (CheckHitNextNode(originSide, ray, objHit, hit) && hit->position.e[axis] >= split) {
            if (CheckHitNextNode(originSide, ray, objHit, hit) && hit->position.e[axis] >= split) {
                return true;
            } else {
                return CheckHitNextNode(oppositeSide, ray, objHit, hit);
            }
        } else {
            originSide   = node->left;
            oppositeSide = node->right;
            // if (CheckHitNextNode(originSide, ray, objHit, hit) && hit->position.e[axis] < split) {
            if (CheckHitNextNode(originSide, ray, objHit, hit) && hit->position.e[axis] < split) {
                return true;
            } else {
                return CheckHitNextNode(oppositeSide, ray, objHit, hit);
            }
        }
    }
}

bool KDTree_HitAt(KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit)
{
    // then we traverse the tree and check for collisions
    return CheckHitNextNode(tree->root, ray, objHit, hit);
}
