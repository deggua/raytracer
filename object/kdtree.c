#include "kdtree.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/cext.h"
#include "common/memory_arena.h"
#include "gfx/utils.h"
#include "object/object.h"

#define LEAF_MAX_LOAD 1
#define NUM_THREADS   4

typedef enum
{
    KD_INTERNAL,
    KD_LEAF,
} KDNodeType;

typedef struct {
    const Object* obj;
    HitInfo       hit[NUM_THREADS];
} KDMetadata;

typedef struct {
    BoundingBox box;
    KDMetadata* metadata;
} KDBB;

typedef struct KDNode KDNode;

typedef struct {
    size_t      len;
    KDMetadata* objs[];
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
    MemoryArena* arena;
    size_t       maxThreads;

    struct {
        size_t      len;
        KDMetadata* arr;
    } metadata;
} KDTree;

static int CompareBB(const KDBB** a, const KDBB** b, VecAxis axis)
{
    const KDBB* bbA = *a;
    const KDBB* bbB = *b;

    float midA = bbA->box.pMin.e[axis] + bbA->box.pMax.e[axis];
    float midB = bbB->box.pMin.e[axis] + bbB->box.pMax.e[axis];

    if (midA < midB) {
        return -1;
    } else if (midA > midB) {
        return 1;
    } else {
        return 0;
    }
}

static int CompareBB_X(const void* a, const void* b)
{
    return CompareBB((const KDBB**)a, (const KDBB**)b, VEC_X);
}

static int CompareBB_Y(const void* a, const void* b)
{
    return CompareBB((const KDBB**)a, (const KDBB**)b, VEC_Y);
}

static int CompareBB_Z(const void* a, const void* b)
{
    return CompareBB((const KDBB**)a, (const KDBB**)b, VEC_Z);
}

static void KDBB_Sort(KDBB* sort[], size_t len, VecAxis axis)
{
    switch (axis) {
        case VEC_X: {
            qsort(sort, len, sizeof(KDBB*), CompareBB_X);
        } break;

        case VEC_Y: {
            qsort(sort, len, sizeof(KDBB*), CompareBB_Y);
        } break;

        case VEC_Z: {
            qsort(sort, len, sizeof(KDBB*), CompareBB_Z);
        } break;

        default: {
            __builtin_unreachable();
        } break;
    };

    return;
}

static KDNode* BuildNode(MemoryArena* arena, KDBB* sort[], size_t len, VecAxis axis);

static KDNode* BuildParentNode(
    MemoryArena* arena,
    KDBB*        leftHalf[],
    size_t       leftLen,
    KDBB*        rightHalf[],
    size_t       rightLen,
    float        partition,
    VecAxis      axis)
{
    KDNode* leftNode = BuildNode(arena, leftHalf, leftLen, (axis + 1) % VEC_LAST);
    if (leftNode == NULL) {
        goto error_lt;
    }

    KDNode* parent = MemoryArena_Malloc(arena, sizeof(*parent));
    if (parent == NULL) {
        goto error_parent;
    }

    KDNode* rightNode = BuildNode(arena, rightHalf, rightLen, (axis + 1) % VEC_LAST);
    if (rightNode == NULL) {
        goto error_gteq;
    }

    parent->type        = KD_INTERNAL;
    parent->inode.left  = leftNode;
    parent->inode.right = rightNode;
    parent->inode.split = partition;

    return parent;

    // TODO: should I bother figuring out how to do deallocations in the arena?
error_parent:
    // free(gteq);
error_gteq:
    // free(lt);
error_lt:
    return NULL;
}

static KDNode* BuildLeafNode(MemoryArena* arena, KDBB* objs[], size_t len)
{
    KDNode* node = MemoryArena_Malloc(arena, sizeof(*node) + len * sizeof(KDMetadata*));
    if (node == NULL) {
        return NULL;
    }

    // initialize the node and copy in the KDMetadata pointers
    node->type     = KD_LEAF;
    node->leaf.len = len;

    for (size_t ii = 0; ii < len; ii++) {
        node->leaf.objs[ii] = objs[ii]->metadata;
        // printf("Put pointer %p in leaf node\n", node->leaf.objs[ii]);
        // printf("Pointer %p has has object pointer %p\n", node->leaf.objs[ii], node->leaf.objs[ii]->obj);
    }

    // printf("Created leaf node with %zu pointers to KDMetadata\n", len);

    return node;
}

static KDNode* BuildNode(MemoryArena* arena, KDBB* sort[], size_t len, VecAxis axis)
{
    if (len <= LEAF_MAX_LOAD) {
        return BuildLeafNode(arena, sort, len);
    }

    KDBB_Sort(sort, len, axis);
    size_t       leftLen      = len / 2;
    const size_t origLeftLen  = leftLen;
    size_t       rightLen     = len - leftLen;
    const size_t origRightLen = rightLen;
    const float  partition    = (sort[leftLen]->box.pMin.e[axis] + sort[leftLen]->box.pMax.e[axis]) / 2.0f;

    KDBB** buffer = calloc(2 * len, sizeof(KDBB*));
    if (buffer == NULL) {
        return NULL;
    }

    KDBB** leftSort  = &buffer[0];
    KDBB** rightSort = &buffer[len];

    memcpy(leftSort, &sort[0], leftLen * sizeof(KDBB*));
    memcpy(rightSort, &sort[leftLen], rightLen * sizeof(KDBB*));

    // copy any in the left the are intersected by the partition to the right half
    for (size_t ii = 0; ii < origLeftLen; ii++) {
        if (within(partition, leftSort[ii]->box.pMin.e[axis], leftSort[ii]->box.pMax.e[axis])) {
            rightSort[rightLen++] = leftSort[ii];
        }
    }

    // copy any in the right the are intersected by the partition to the left half
    for (size_t ii = 0; ii < origRightLen; ii++) {
        if (within(partition, rightSort[ii]->box.pMin.e[axis], rightSort[ii]->box.pMax.e[axis])) {
            leftSort[leftLen++] = rightSort[ii];
        }
    }

    // TODO: there is a problem with the construction that results in inf recursive loop, not sure why, going to just
    // make it so if either rightLen/leftLen == len then just make a LeafNode
    KDNode* node;
    if (leftLen == len || rightLen == len) {
        free(buffer);
        node = BuildLeafNode(arena, sort, len);
    } else {
        node = BuildParentNode(arena, leftSort, leftLen, rightSort, rightLen, partition, axis);
        free(buffer);
    }

    return node;
}

static KDNode* BuildKDTree(MemoryArena* arena, KDBB boxes[], size_t len)
{
    // we need a buffer of pointers to the objects BB's
    KDBB** pBoxes = calloc(len, sizeof(KDBB*));
    if (pBoxes == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        pBoxes[ii] = &boxes[ii];
    }

    return BuildNode(arena, pBoxes, len, VEC_X);
}

static KDBB* CreateKDBBs(const KDMetadata metadata[], size_t len)
{
    KDBB* boxes = calloc(len, sizeof(KDBB));
    if (boxes == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        Surface_BoundedBy(&metadata[ii].obj->surface, &boxes[ii].box);
        boxes[ii].metadata = (KDMetadata*)&metadata[ii];
    }

    return boxes;
}

static KDMetadata* CreateKDMetadata(const Object objs[], size_t len)
{
    KDMetadata* metadata = calloc(len, sizeof(KDMetadata));
    if (metadata == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        metadata[ii].obj = &objs[ii];
        for (size_t jj = 0; jj < NUM_THREADS; jj++) {
            metadata[ii].hit[jj].tIntersect = NAN;
        }
    }

    return metadata;
}

KDTree* KDTree_New(const Object objs[], size_t len, size_t numThreads)
{
    KDTree* tree = calloc(1, sizeof(KDTree));
    if (tree == NULL) {
        goto error_KDTree;
    }

    if (numThreads != NUM_THREADS) {
        // TODO: not sure how to handle this, arrays with FAMs are hard to work with/iterate over
        // it can be worked around, but the solution is ugly
        return NULL;
    }
    tree->maxThreads = numThreads;

    // max number of inodes                     = len - 1
    // max number of leaf nodes                 = len
    // max number of KDMetadata* per leaf node  = len
    const size_t alignment = 16;
    const size_t nodeMem   = (2 * len - 1) * alignto(sizeof(KDNode), alignment - 1);
    const size_t metaMem   = alignto(len * sizeof(KDMetadata*), alignment - 1);
    const size_t totalMem  = alignto(nodeMem + metaMem, alignment - 1);

    MemoryArena* arena = MemoryArena_New(totalMem, alignment);
    if (arena == NULL) {
        goto error_Arena;
    }

    tree->arena = arena;

    tree->metadata.arr = CreateKDMetadata(objs, len);
    tree->metadata.len = len;
    if (tree->metadata.arr == NULL) {
        goto error_Metadata;
    }

    KDBB* boxes = CreateKDBBs(tree->metadata.arr, len);
    if (boxes == NULL) {
        goto error_KDBBs;
    }

    tree->root = BuildKDTree(arena, boxes, len);
    if (tree->root == NULL) {
        goto error_Root;
    }
    free(boxes);

    return tree;

error_Root:
    free(boxes);
error_KDBBs:
    free(tree->metadata.arr);
error_Metadata:
    MemoryArena_Delete(arena);
error_Arena:
    free(tree);
error_KDTree:
    return NULL;
}

void KDTree_Delete(KDTree* tree)
{
    MemoryArena_Delete(tree->arena);
    free(tree->metadata.arr);
    free(tree);
}

static bool CheckHitLeafNode(
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    VecAxis       axis,
    const size_t  threadNum);

static bool CheckHitInternalNode(
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    VecAxis       axis,
    const size_t  threadNum);

static bool CheckHitNextNode(
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    VecAxis       axis,
    const size_t  threadNum)
{
    switch (node->type) {
        case KD_INTERNAL: {
            return CheckHitInternalNode(node, ray, objHit, hit, axis, threadNum);
        } break;

        case KD_LEAF: {
            return CheckHitLeafNode(node, ray, objHit, hit, axis, threadNum);
        } break;
    };

    // TODO: try to enable and see if it breaks code gen
    __builtin_unreachable();
    return false;
}

static bool CheckHitLeafNode(
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    VecAxis       axis,
    const size_t  threadNum)
{
    // TODO: not sure if there's any point to this, my thought process is that we basically never change args
    // so maybe this helps the compiler optimize if all the recursive functions have the same args? No idea
    (void)axis;

    HitInfo       hitClosest = {.tIntersect = INF};
    const Object* objClosest = NULL;

    for (size_t ii = 0; ii < node->leaf.len; ii++) {
        HitInfo*      hitTemp = &node->leaf.objs[ii]->hit[threadNum];
        const Object* objTemp = NULL;

        if (!isnan(node->leaf.objs[ii]->hit[threadNum].tIntersect)) {
            // we already hit this object so just pull the intersect info out
            objTemp = node->leaf.objs[ii]->obj;
        } else if (Surface_HitAt(&node->leaf.objs[ii]->obj->surface, ray, 0.001f, INF, hitTemp)) {
            objTemp = node->leaf.objs[ii]->obj;
        }

        if (objTemp != NULL && hitTemp->tIntersect < hitClosest.tIntersect) {
            hitClosest = *hitTemp;
            objClosest = objTemp;
        }
    }

    if (objClosest != NULL) {
        *hit    = hitClosest;
        *objHit = (Object*)objClosest;
        return true;
    }

    return false;
}

static bool CheckHitInternalNode(
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    VecAxis       axis,
    const size_t  threadNum)
{
    const float split            = node->inode.split;
    const float epsilonParallel  = 0.0001f;
    const float epsilonIntersect = 0.001f;

    if (fabsf(ray->dir.e[axis]) < epsilonParallel) {
        // ray parallel to plane, check the origin to see which side ray falls on
        // TODO: is this correct when the origin is in the plane?
        if (ray->origin.e[axis] >= split) {
            return CheckHitNextNode(node->inode.right, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
        } else {
            return CheckHitNextNode(node->inode.left, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
        }
    }

    float tIntersect = (split - ray->origin.e[axis]) / ray->dir.e[axis];
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t, check the origin to see which side ray falls on
        if (ray->origin.e[axis] > split) {
            return CheckHitNextNode(node->inode.right, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
        } else if (ray->origin.e[axis] < split) {
            return CheckHitNextNode(node->inode.left, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
        } else {
            // ray intersects the plane at the origin, eval the ray later and see what side it's on
            // TODO: couldn't we just look at the direction? or just compute the sum of the origin + direction?
            // OPTIMIZE
            float rayAt = Ray_At(ray, 1.0f).e[axis];
            if (rayAt >= split) {
                return CheckHitNextNode(node->inode.right, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
            } else {
                return CheckHitNextNode(node->inode.left, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side. we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite since the ray will
        // hit objects on the origin side before objects on the opposite side
        const KDNode* originSide;
        const KDNode* oppositeSide;

        if (ray->origin.e[axis] >= split) {
            originSide   = node->inode.right;
            oppositeSide = node->inode.left;
            if (CheckHitNextNode(originSide, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum)
                && hit->position.e[axis] >= split) {
                return true;
            } else {
                return CheckHitNextNode(oppositeSide, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
            }
        } else {
            originSide   = node->inode.right;
            oppositeSide = node->inode.left;
            if (CheckHitNextNode(originSide, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum)
                && hit->tIntersect < split) {
                return true;
            } else {
                return CheckHitNextNode(oppositeSide, ray, objHit, hit, (axis + 1) % VEC_LAST, threadNum);
            }
        }
    }
}

bool KDTree_HitAt(KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit, const size_t threadNum)
{
    // first we NAN out the metadata
    for (size_t ii = 0; ii < tree->metadata.len; ii++) {
        tree->metadata.arr[ii].hit[threadNum].tIntersect = NAN;
    }

    // then we traverse the tree and check for collisions
    return CheckHitNextNode(tree->root, ray, objHit, hit, VEC_X, threadNum);
}
