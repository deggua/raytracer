#include "kdtree.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math/math.h"
#include "world/object.h"

/* ---  KD-Tree Metaparameters --- */
// Threshold for which a leaf node will be automatically created. Lower values result in greater tree depth and sparser
// leaves, which can be better or worse depending on scene complexity
// Range: [2, INF)
static const size_t MinLeafLoad = 4;

// The number of positions to compute the SAH along each axis. Large values result in a finer resolution SAH search
// which can yield a better tree at the cost of time to construct the tree.
// Range: [2, INF)
static const size_t NumBuckets = 32;

// The cost of a triangle intersection. Should be > 1, and is only relevant in relation to the TraversalCost
// Range: (1, INF)
static const f32 IntersectCost = 1.0f;

// A bonus weight towards putting objects in the right side of the tree. Because the right child is directly adjacent to
// its parent, the cost of right traversal is less because it should be in cache
// Range: [0.9, 1]
static const f32 RightNodeRelativeCost = 0.95f;

// A bonus weight towards having empty tree regions. Helps with early termination during traversal.
// Range: [0, 1]
static const f32 EmptyBonus = 0.5f;

// These shouldn't be tuned directly, instead change their relative parameter above
static const f32 TraversalCost        = 1.0f;
static const f32 LeftNodeRelativeCost = 1.0f + (1.0f - RightNodeRelativeCost);

typedef int64_t ssize_t;

/* --- TODOs --- */
// Debugging modifications:
// TODO: track avg intersection tests / ray to better determine what metaparmeter changes do to the traversal provide

typedef enum {
    KD_INTERNAL_X = AXIS_X,
    KD_INTERNAL_Y = AXIS_Y,
    KD_INTERNAL_Z = AXIS_Z,
    KD_LEAF       = 3,
} KDNodeType;

typedef struct KDBB {
    BoundingBox   box;
    const Object* obj;
} KDBB;

#define TEMPLATE_TYPE Object*
#define TEMPLATE_NAME ObjectPtr
#include "templates/vector.h"

#define TEMPLATE_TYPE KDBB*
#define TEMPLATE_NAME KDBBPtr
#include "templates/vector.h"

#define TEMPLATE_TYPE KDBB
#include "templates/vector.h"

typedef struct {
    struct {
        u32     : 2;
        u32 len : 30;
    };

    u32 objIndex;
} KDLeaf;

static_assert(sizeof(KDLeaf) == 8, "sizeof(KDLeaf) != 8");

typedef struct {
    struct {
        u32           : 2;
        u32 leftIndex : 30;
    };

    f32 split;
    u8  right[];
} KDInternal;

static_assert(sizeof(KDInternal) == 8, "sizeof(KDInternal) != 8");

typedef union KDNode {
    struct {
        u32 type : 2;
        u32      : 30;
    };

    KDInternal inode;
    KDLeaf     leaf;
} KDNode;

static_assert(sizeof(KDNode) == 8, "sizeof(KDNode) != 8");

#define TEMPLATE_TYPE KDNode
#include "templates/vector.h"

typedef struct KDTree {
    Vector(KDNode) * nodes;
    Vector(ObjectPtr) * objPtrs;
    BoundingBox worldBox;
    ssize_t     rootIndex;
} KDTree;

static ssize_t BuildNode(KDTree* tree, const Vector(KDBBPtr) * vec, BoundingBox container, size_t depth);

static ssize_t BuildParentNode(
    KDTree* tree,
    const Vector(KDBBPtr) * leftVec,
    BoundingBox leftContainer,
    const Vector(KDBBPtr) * rightVec,
    BoundingBox rightContainer,
    f32         partition,
    Axis        axis,
    size_t      depth)
{
    size_t parentIndex = tree->nodes->length;

    if (!Vector_ExtendBy(KDNode)(tree->nodes, 1)) {
        goto error_parent;
    }

    // construct right node after parent, causes the next node to be at the index immediately after
    // parent's index (ie if you have the KDNode* to parent, parent + 1 == right child)
    ssize_t rightNode = BuildNode(tree, rightVec, rightContainer, depth);

    if (rightNode < 0) {
        goto error_gteq;
    }

    ssize_t leftNode = BuildNode(tree, leftVec, leftContainer, depth);

    if (leftNode < 0) {
        goto error_lt;
    }

    KDNode* parent          = &tree->nodes->at[parentIndex];
    parent->type            = (KDNodeType)axis;
    parent->inode.leftIndex = leftNode;
    parent->inode.split     = partition;

    return parentIndex;

error_parent:
error_gteq:
error_lt:
    return -1;
}

static ssize_t BuildLeafNode(KDTree* tree, const Vector(KDBBPtr) * vec)
{
    size_t nodeIndex = tree->nodes->length;

    if (!Vector_ExtendBy(KDNode)(tree->nodes, 1)) {
        return -1;
    }

    KDNode* node   = &tree->nodes->at[nodeIndex];
    node->type     = KD_LEAF;
    node->leaf.len = vec->length;

    ssize_t firstObjIndex = tree->objPtrs->length;

    for (size_t ii = 0; ii < vec->length; ii++) {
        if (!Vector_Push(ObjectPtr)(tree->objPtrs, &vec->at[ii]->obj)) {
            return -1;
        }
    }

    node->leaf.objIndex = firstObjIndex;

    return nodeIndex;
}

static f32 SurfaceArea(BoundingBox box)
{
    const f32 xDim = box.max.x - box.min.x;
    const f32 yDim = box.max.y - box.min.y;
    const f32 zDim = box.max.z - box.min.z;

    return 2 * xDim * yDim + 2 * xDim * zDim + 2 * yDim * zDim;
}

static BoundingBox BoxBoundingAll(const Vector(KDBB) * vec)
{
    KDBB*  kdbbs = vec->at;
    size_t len   = vec->length;

    BoundingBox box = {
        .min = kdbbs[0].box.min,
        .max = kdbbs[0].box.max,
    };

    for (size_t ii = 1; ii < len; ii++) {
        for (Axis axis = AXIS_X; axis <= AXIS_Z; axis++) {
            if (kdbbs[ii].box.min.elem[axis] < box.min.elem[axis]) {
                box.min.elem[axis] = kdbbs[ii].box.min.elem[axis];
            }

            if (kdbbs[ii].box.max.elem[axis] > box.max.elem[axis]) {
                box.max.elem[axis] = kdbbs[ii].box.max.elem[axis];
            }
        }
    }

    return box;
}

typedef struct {
    BoundingBox left;
    BoundingBox right;
} BoundingBoxPair;

static BoundingBoxPair SplitBox(BoundingBox box, f32 split, Axis axis)
{
    BoundingBoxPair splitBoxes = {
        .left  = box,
        .right = box,
    };
    splitBoxes.left.max.elem[axis]  = split;
    splitBoxes.right.min.elem[axis] = split;

    return splitBoxes;
}

static f32 ComputeSplitSAH(const Vector(KDBBPtr) * vec, f32 split, Axis axis, BoundingBox parent)
{
    const f32 parentSA = SurfaceArea(parent);

    KDBB** kdbbs = vec->at;

    BoundingBoxPair boxPair    = SplitBox(parent, split, axis);
    size_t          leftPrims  = 0;
    size_t          rightPrims = 0;

    for (size_t ii = 0; ii < vec->length; ii++) {
        const BoundingBox* box = &kdbbs[ii]->box;

        if (box->max.elem[axis] < split) {
            leftPrims += 1;
        } else if (box->min.elem[axis] > split) {
            rightPrims += 1;
        } else {
            leftPrims += 1;
            rightPrims += 1;
        }
    }

    f32 leftCost   = (SurfaceArea(boxPair.left) / parentSA) * leftPrims * IntersectCost * LeftNodeRelativeCost;
    f32 rightCost  = (SurfaceArea(boxPair.right) / parentSA) * rightPrims * IntersectCost * RightNodeRelativeCost;
    f32 emptyBonus = (leftPrims == 0 || rightPrims == 0) ? EmptyBonus : 0;

    return TraversalCost + (1.0f - emptyBonus) * (leftCost + rightCost);
}

static ssize_t BuildNode(KDTree* tree, const Vector(KDBBPtr) * vec, BoundingBox container, size_t depth)
{
    const size_t maxVecLen = (1 << 30) - 1;

    // prevent blowing out the max index silently (would cause inf render time)
    if (tree->nodes->length > maxVecLen || tree->objPtrs->length > maxVecLen) {
        printf("Too many objects allocated to vector in %s\n", __func__);
        exit(EXIT_FAILURE);
    }

    if (vec->length <= MinLeafLoad || depth == 0) {
        return BuildLeafNode(tree, vec);
    }

    f32  bestSplit = 0.0f;
    Axis bestAxis  = AXIS_X;
    f32  bestSAH   = INF;

    for (Axis axis = AXIS_X; axis <= AXIS_Z; axis++) {
        f32 stride = (container.max.elem[axis] - container.min.elem[axis]) / NumBuckets;

        for (f32 bucket = container.min.elem[axis] + stride; bucket <= container.max.elem[axis] - stride;
             bucket += stride) {
            f32 split = bucket;
            f32 SAH   = ComputeSplitSAH(vec, split, axis, container);

            if (SAH < bestSAH) {
                bestSAH   = SAH;
                bestAxis  = axis;
                bestSplit = split;
            }
        }
    }

    f32 parentSAH = vec->length * IntersectCost;

    if (parentSAH <= bestSAH) {
        // cost of best split outweighs just putting everything in a leaf
        return BuildLeafNode(tree, vec);
    } else {
        // cost of best split is better than cost of total, split them up
        // TODO: use updated vector template to avoid allocating vector objects on the heap
        Vector(KDBBPtr)* leftVec = Vector_New(KDBBPtr)();
        if (leftVec == NULL) {
            goto error_LeftVector;
        }

        Vector(KDBBPtr)* rightVec = Vector_New(KDBBPtr)();
        if (rightVec == NULL) {
            goto error_RightVector;
        }

        if (!Vector_Reserve(KDBBPtr)(leftVec, vec->length)) {
            goto error_Reserve;
        }

        if (!Vector_Reserve(KDBBPtr)(rightVec, vec->length)) {
            goto error_Reserve;
        }

        for (size_t ii = 0; ii < vec->length; ii++) {
            const BoundingBox* box = &vec->at[ii]->box;

            if (box->max.elem[bestAxis] < bestSplit) {
                if (!Vector_Push(KDBBPtr)(leftVec, (const KDBB**)&vec->at[ii])) {
                    goto error_Push;
                }
            } else if (box->min.elem[bestAxis] > bestSplit) {
                if (!Vector_Push(KDBBPtr)(rightVec, (const KDBB**)&vec->at[ii])) {
                    goto error_Push;
                }
            } else {
                if (!Vector_Push(KDBBPtr)(leftVec, (const KDBB**)&vec->at[ii])) {
                    goto error_Push;
                }

                if (!Vector_Push(KDBBPtr)(rightVec, (const KDBB**)&vec->at[ii])) {
                    goto error_Push;
                }
            }
        }

        BoundingBoxPair pair = SplitBox(container, bestSplit, bestAxis);
        ssize_t         nodeIndex
            = BuildParentNode(tree, leftVec, pair.left, rightVec, pair.right, bestSplit, bestAxis, depth - 1);

        if (nodeIndex < 0) {
            goto error_BuildParent;
        }

        Vector_Delete(KDBBPtr)(leftVec);
        Vector_Delete(KDBBPtr)(rightVec);

        return nodeIndex;

error_BuildParent:
error_Push:
error_Reserve:
        Vector_Delete(KDBBPtr)(rightVec);
error_RightVector:
        Vector_Delete(KDBBPtr)(leftVec);
error_LeftVector:
        return -1;
    }
}

static ssize_t BuildKDTree(KDTree* tree, const Vector(KDBB) * boxes, size_t maxDepth)
{
    // we need a buffer of pointers to the objects BB's
    Vector(KDBBPtr)* vec = Vector_New(KDBBPtr)();

    if (vec == NULL) {
        goto error_VectorNew;
    }

    if (!Vector_Reserve(KDBBPtr)(vec, boxes->length)) {
        goto error_VectorReserve;
    }

    for (size_t ii = 0; ii < boxes->length; ii++) {
        KDBB* ptr = &boxes->at[ii];

        if (!Vector_Push(KDBBPtr)(vec, (const KDBB**)&ptr)) {
            goto error_VectorPush;
        }
    }

    ssize_t rootIndex = BuildNode(tree, vec, tree->worldBox, maxDepth);

    if (rootIndex < 0) {
        goto error_Root;
    }

    Vector_Delete(KDBBPtr)(vec);
    return rootIndex;

error_Root:
error_VectorPush:
error_VectorReserve:
    Vector_Delete(KDBBPtr)(vec);
error_VectorNew:
    return -1;
}

static Vector(KDBB) * CreateKDBBs(const Object objs[], size_t len)
{
    Vector(KDBB)* vec = Vector_New(KDBB)();

    if (vec == NULL) {
        goto error_VectorKDBBs;
    }

    if (!Vector_Reserve(KDBB)(vec, len)) {
        goto error_VectorKDBBsReserve;
    }

    for (size_t ii = 0; ii < len; ii++) {
        KDBB temp;
        temp.obj = &objs[ii];
        Surface_BoundedBy(&objs[ii].surface, &temp.box);

        if (!Vector_Push(KDBB)(vec, &temp)) {
            goto error_VectorKDBBsPush;
        }
    }

    return vec;

error_VectorKDBBsPush:
error_VectorKDBBsReserve:
    Vector_Delete(KDBB)(vec);
error_VectorKDBBs:
    return NULL;
}

KDTree* KDTree_New(const Object objs[], size_t len)
{
    // NOTE: this is fine tuned
    const size_t maxDepth = (size_t)(8.0 + 1.8 * log2(len));

    KDTree* tree = calloc(1, sizeof(KDTree));

    if (tree == NULL) {
        goto error_KDTree;
    }

    tree->nodes = Vector_New(KDNode)();

    if (tree->nodes == NULL) {
        goto error_NodeVector;
    }

    // practical limits on the number of nodes and objects due to the index being 30 bits
    // this prevents performance penalties as a result of poor malloc implementations
    size_t nodes_upper_bound = (1ull << 30) - 1;
    size_t objs_upper_bound  = (1ull << 30) - 1;

    if (!Vector_Reserve(KDNode)(tree->nodes, nodes_upper_bound)) {
        goto error_NodeVectorReserve;
    }

    tree->objPtrs = Vector_New(ObjectPtr)();

    if (tree->objPtrs == NULL) {
        goto error_ObjVector;
    }

    if (!Vector_Reserve(ObjectPtr)(tree->objPtrs, objs_upper_bound)) {
        goto error_ObjVectorReserve;
    }

    Vector(KDBB)* boxes = CreateKDBBs(objs, len);

    if (boxes == NULL) {
        goto error_KDBBs;
    }

    tree->worldBox  = BoxBoundingAll(boxes);
    tree->rootIndex = BuildKDTree(tree, boxes, maxDepth);

    if (tree->rootIndex < 0) {
        goto error_Root;
    }

    Vector_Delete(KDBB)(boxes);
    return tree;

error_Root:
    Vector_Delete(KDBB)(boxes);
error_KDBBs:
error_ObjVectorReserve:
    Vector_Delete(ObjectPtr)(tree->objPtrs);
error_ObjVector:
error_NodeVectorReserve:
    Vector_Delete(KDNode)(tree->nodes);
error_NodeVector:
    free(tree);
error_KDTree:
    return NULL;
}

void KDTree_Delete(KDTree* tree)
{
    Vector_Delete(KDNode)(tree->nodes);
    Vector_Delete(ObjectPtr)(tree->objPtrs);
    free(tree);
}

static bool
CheckHitLeafNode(const KDTree* tree, const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit, f32 tMax);

static bool CheckHitInternalNode(
    const KDTree*     tree,
    const KDInternal* node,
    const Ray*        ray,
    Object**          objHit,
    HitInfo*          hit,
    Axis              axis,
    f32               tMax);

static bool
CheckHitNextNode(const KDTree* tree, const KDNode* node, const Ray* ray, Object** objHit, HitInfo* hit, f32 tMax)
{
    switch (node->type) {
        case KD_INTERNAL_X:
        case KD_INTERNAL_Y:
        case KD_INTERNAL_Z: {
            return CheckHitInternalNode(tree, &node->inode, ray, objHit, hit, (Axis)node->type, tMax);
        }

        case KD_LEAF: {
            return CheckHitLeafNode(tree, &node->leaf, ray, objHit, hit, tMax);
        } break;
    };

    return false;
}

static bool
CheckHitLeafNode(const KDTree* tree, const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit, f32 tMax)
{
    HitInfo       hitClosest = {.tIntersect = INF};
    const Object* objClosest = NULL;

    for (size_t ii = 0; ii < leaf->len; ii++) {
        HitInfo       hitCur;
        const Object* objCur = tree->objPtrs->at[leaf->objIndex + ii];

        if (Surface_HitAt(&objCur->surface, ray, 0.001f, tMax, &hitCur)) {
            if (hitCur.tIntersect < hitClosest.tIntersect) {
                objClosest = objCur;
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

static bool CheckHitInternalNode(
    const KDTree*     tree,
    const KDInternal* node,
    const Ray*        ray,
    Object**          objHit,
    HitInfo*          hit,
    Axis              axis,
    f32               tMax)
{
    const f32 split            = node->split;
    const f32 epsilonParallel  = 0.0001f;
    const f32 epsilonIntersect = 0.001f;

    const KDNode* left  = &tree->nodes->at[node->leftIndex];
    const KDNode* right = (KDNode*)node->right;

    if (unlikely(fabsf(ray->dir.elem[axis]) < epsilonParallel)) {
        // ray parallel to plane, check the origin to see which side ray falls on
        if (ray->origin.elem[axis] >= split) {
            return CheckHitNextNode(tree, right, ray, objHit, hit, tMax);
        } else {
            return CheckHitNextNode(tree, left, ray, objHit, hit, tMax);
        }
    }

    f32 tIntersect = split * ray->cache.invDir.elem[axis] - ray->cache.originDivDir.elem[axis];
    f32 tMaxNew    = minf(tMax, tIntersect);

    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t, check the origin to see which side ray falls on
        if (likely(ray->origin.elem[axis] > split)) {
            return CheckHitNextNode(tree, right, ray, objHit, hit, tMax);
        } else if (likely(ray->origin.elem[axis] < split)) {
            return CheckHitNextNode(tree, left, ray, objHit, hit, tMax);
        } else {
            // ray intersects the plane at the origin, eval the ray later and see what side it's on
            f32 rayAt = Ray_At(ray, 1.0f).elem[axis];

            if (rayAt >= split) {
                return CheckHitNextNode(tree, right, ray, objHit, hit, tMax);
            } else {
                return CheckHitNextNode(tree, left, ray, objHit, hit, tMax);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side. we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite since the ray will
        // hit objects on the origin side before objects on the opposite side
        const KDNode* originSide;
        const KDNode* oppositeSide;

        if (ray->origin.elem[axis] >= split) {
            originSide   = right;
            oppositeSide = left;

            if (CheckHitNextNode(tree, originSide, ray, objHit, hit, tMaxNew)) {
                // the object hit needs to lie on the right side of the plane
                return true;
            } else if (tMaxNew == tIntersect) {
                return CheckHitNextNode(tree, oppositeSide, ray, objHit, hit, tMax);
            } else {
                return false;
            }
        } else {
            originSide   = left;
            oppositeSide = right;

            if (CheckHitNextNode(tree, originSide, ray, objHit, hit, tMaxNew)) {
                // the object hit needs to lie on the left side of the plane
                return true;
            } else if (tMaxNew == tIntersect) {
                return CheckHitNextNode(tree, oppositeSide, ray, objHit, hit, tMax);
            } else {
                return false;
            }
        }
    }
}

bool KDTree_HitAt(const KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit)
{
    // then we traverse the tree and check for collisions
    KDNode* root = &tree->nodes->at[tree->rootIndex];
    return CheckHitNextNode(tree, root, ray, objHit, hit, INF);
}
