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
    BoundingBox box;
    Object*     obj;
} KDBB;

typedef struct {
    BoundingBox left;
    BoundingBox right;
} BoundingBoxPair;

#define Vector_Type       Object*
#define Vector_Type_Alias ObjectPtr
#include "ctl/containers/vector.h"

#define Vector_Type       KDBB*
#define Vector_Type_Alias KDBBPtr
#include "ctl/containers/vector.h"

#define Vector_Type KDBB
#include "ctl/containers/vector.h"

typedef struct {
    struct {
        u32     : 2;
        u32 len : 30;
    };

    u32 objIndex;
} KDLeaf;

static_assert_decl(sizeof(KDLeaf) == 8);

typedef struct {
    struct {
        u32           : 2;
        u32 leftIndex : 30;
    };

    f32 split;
    u8  right[];
} KDInternal;

static_assert_decl(sizeof(KDInternal) == 8);

typedef union KDNode {
    struct {
        u32 type : 2;
        u32      : 30;
    };

    KDInternal inode;
    KDLeaf     leaf;
} KDNode;

static_assert_decl(sizeof(KDNode) == 8);

#define Vector_Type KDNode
#include "ctl/containers/vector.h"

typedef struct KDTree {
    Vector(KDNode)*    nodes;
    Vector(ObjectPtr)* objPtrs;
    BoundingBox        worldBox;
    ssize_t            rootIndex;
} KDTree;

intern ssize_t BuildNode(KDTree* tree, Vector(KDBBPtr)* vect, BoundingBox container, size_t depth);

intern ssize_t BuildParentNode(
    KDTree*          tree,
    Vector(KDBBPtr)* leftVect,
    BoundingBox      leftContainer,
    Vector(KDBBPtr)* rightVect,
    BoundingBox      rightContainer,
    f32              partition,
    Axis             axis,
    size_t           depth)
{
    size_t parentIndex = tree->nodes->length;

    if (!Vector_ExtendBy(tree->nodes, 1)) {
        goto error_parent;
    }

    // construct right node after parent, causes the next node to be at the index immediately after
    // parent's index (ie if you have the KDNode* to parent, parent + 1 == right child)
    ssize_t rightNode = BuildNode(tree, rightVect, rightContainer, depth);
    if (rightNode < 0) {
        goto error_gteq;
    }

    ssize_t leftNode = BuildNode(tree, leftVect, leftContainer, depth);
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

intern ssize_t BuildLeafNode(KDTree* tree, Vector(KDBBPtr)* vect)
{
    size_t nodeIndex = tree->nodes->length;

    if (!Vector_ExtendBy(tree->nodes, 1)) {
        return -1;
    }

    KDNode* node   = &tree->nodes->at[nodeIndex];
    node->type     = KD_LEAF;
    node->leaf.len = vect->length;

    ssize_t firstObjIndex = tree->objPtrs->length;

    for (size_t ii = 0; ii < vect->length; ii++) {
        if (!Vector_Push(tree->objPtrs, vect->at[ii]->obj)) {
            return -1;
        }
    }

    node->leaf.objIndex = firstObjIndex;

    return nodeIndex;
}

intern f32 SurfaceArea(BoundingBox box)
{
    f32 xDim = box.max.x - box.min.x;
    f32 yDim = box.max.y - box.min.y;
    f32 zDim = box.max.z - box.min.z;

    return 2 * xDim * yDim + 2 * xDim * zDim + 2 * yDim * zDim;
}

intern BoundingBox BoxBoundingAll(Vector(KDBB)* vect)
{
    KDBB*  kdbbs = vect->at;
    size_t len   = vect->length;

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

intern BoundingBoxPair SplitBox(BoundingBox box, f32 split, Axis axis)
{
    BoundingBoxPair splitBoxes = {
        .left  = box,
        .right = box,
    };
    splitBoxes.left.max.elem[axis]  = split;
    splitBoxes.right.min.elem[axis] = split;

    return splitBoxes;
}

intern f32 ComputeSplitSAH(Vector(KDBBPtr)* vect, f32 split, Axis axis, BoundingBox parent)
{
    f32 parentSA = SurfaceArea(parent);

    KDBB** kdbbs = vect->at;

    BoundingBoxPair boxPair    = SplitBox(parent, split, axis);
    size_t          leftPrims  = 0;
    size_t          rightPrims = 0;

    for (size_t ii = 0; ii < vect->length; ii++) {
        BoundingBox* box = &kdbbs[ii]->box;

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

intern ssize_t BuildNode(KDTree* tree, Vector(KDBBPtr)* vect, BoundingBox container, size_t depth)
{
    size_t maxVecLen = (1 << 30) - 1;

    // prevent blowing out the max index silently (would cause inf render time)
    if (tree->nodes->length > maxVecLen || tree->objPtrs->length > maxVecLen) {
        ABORT("Too many objects allocated to vector");
    }

    if (vect->length <= MinLeafLoad || depth == 0) {
        return BuildLeafNode(tree, vect);
    }

    f32  bestSplit = 0.0f;
    Axis bestAxis  = AXIS_X;
    f32  bestSAH   = INF;

    for (Axis axis = AXIS_X; axis <= AXIS_Z; axis++) {
        f32 stride = (container.max.elem[axis] - container.min.elem[axis]) / NumBuckets;

        for (f32 bucket = container.min.elem[axis] + stride; bucket <= container.max.elem[axis] - stride;
             bucket += stride) {
            f32 split = bucket;
            f32 SAH   = ComputeSplitSAH(vect, split, axis, container);

            if (SAH < bestSAH) {
                bestSAH   = SAH;
                bestAxis  = axis;
                bestSplit = split;
            }
        }
    }

    f32 parentSAH = vect->length * IntersectCost;

    if (parentSAH <= bestSAH) {
        // cost of best split outweighs just putting everything in a leaf
        return BuildLeafNode(tree, vect);
    } else {
        // cost of best split is better than cost of total, split them up

        Vector(KDBBPtr) leftVect;
        Vector(KDBBPtr) rightVect;

        if (!Vector_Init(&leftVect, vect->length)) {
            goto error_LeftVector;
        }

        if (!Vector_Init(&rightVect, vect->length)) {
            goto error_RightVector;
        }

        for (size_t ii = 0; ii < vect->length; ii++) {
            BoundingBox* box = &vect->at[ii]->box;

            if (box->max.elem[bestAxis] < bestSplit) {
                if (!Vector_Push(&leftVect, &vect->at[ii])) {
                    goto error_Push;
                }
            } else if (box->min.elem[bestAxis] > bestSplit) {
                if (!Vector_Push(&rightVect, &vect->at[ii])) {
                    goto error_Push;
                }
            } else {
                if (!Vector_Push(&leftVect, &vect->at[ii]) || !Vector_Push(&rightVect, &vect->at[ii])) {
                    goto error_Push;
                }
            }
        }

        BoundingBoxPair pair = SplitBox(container, bestSplit, bestAxis);
        ssize_t         nodeIndex
            = BuildParentNode(tree, &leftVect, pair.left, &rightVect, pair.right, bestSplit, bestAxis, depth - 1);

        if (nodeIndex < 0) {
            goto error_BuildParent;
        }

        Vector_Uninit(&leftVect);
        Vector_Uninit(&rightVect);

        return nodeIndex;

error_BuildParent:
error_Push:
        Vector_Uninit(&rightVect);
error_RightVector:
        Vector_Uninit(&leftVect);
error_LeftVector:
        return -1;
    }
}

intern ssize_t BuildKDTree(KDTree* tree, Vector(KDBB)* boxes, size_t maxDepth)
{
    // we need a buffer of pointers to the objects BB's
    Vector(KDBBPtr) vect;
    if (!Vector_Init(&vect, boxes->length)) {
        goto error_VectorNew;
    }

    for (size_t ii = 0; ii < boxes->length; ii++) {
        if (!Vector_Push(&vect, &boxes->at[ii])) {
            goto error_VectorPush;
        }
    }

    ssize_t rootIndex = BuildNode(tree, &vect, tree->worldBox, maxDepth);

    if (rootIndex < 0) {
        goto error_Root;
    }

    Vector_Uninit(&vect);
    return rootIndex;

error_Root:
error_VectorPush:
    Vector_Delete(&vect);
error_VectorNew:
    return -1;
}

intern Vector(KDBB)* CreateKDBBs(Object* objs, size_t len)
{
    Vector(KDBB)* vect = Vector_New(KDBB)(len);
    if (vect == NULL) {
        goto error_VectorKDBBs;
    }

    for (size_t ii = 0; ii < len; ii++) {
        KDBB temp;
        temp.obj = &objs[ii];
        Surface_BoundedBy(&objs[ii].surface, &temp.box);

        if (!Vector_Push(vect, &temp)) {
            goto error_VectorKDBBsPush;
        }
    }

    return vect;

error_VectorKDBBsPush:
    Vector_Delete(vect);
error_VectorKDBBs:
    return NULL;
}

KDTree* KDTree_New(Object* objs, size_t len)
{
    // NOTE: this is fine tuned
    size_t maxDepth = (size_t)(8.0 + 1.8 * log2(len));

    // practical limits on the number of nodes and objects due to the index being 30 bits
    // this prevents performance penalties as a result of poor malloc implementations
    size_t nodes_upper_bound = (1ull << 30) - 1;
    size_t objs_upper_bound  = (1ull << 30) - 1;

    KDTree* tree = calloc(1, sizeof(KDTree));
    if (tree == NULL) {
        goto error_KDTree;
    }

    tree->nodes = Vector_New(KDNode)(nodes_upper_bound);
    if (tree->nodes == NULL) {
        goto error_NodeVector;
    }

    tree->objPtrs = Vector_New(ObjectPtr)(objs_upper_bound);
    if (tree->objPtrs == NULL) {
        goto error_ObjVector;
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

    Vector_Delete(boxes);
    return tree;

error_Root:
    Vector_Delete(boxes);
error_KDBBs:
    Vector_Delete(tree->objPtrs);
error_ObjVector:
    Vector_Delete(tree->nodes);
error_NodeVector:
    free(tree);
error_KDTree:
    return NULL;
}

void KDTree_Delete(KDTree* tree)
{
    Vector_Delete(tree->nodes);
    Vector_Delete(tree->objPtrs);
    free(tree);
}

intern bool CheckHitLeafNode(KDTree* tree, KDLeaf* leaf, Ray* ray, Object** objHit, HitInfo* hit, f32 tMax);

intern bool
CheckHitInternalNode(KDTree* tree, KDInternal* node, Ray* ray, Object** objHit, HitInfo* hit, Axis axis, f32 tMax);

intern bool CheckHitNextNode(KDTree* tree, KDNode* node, Ray* ray, Object** objHit, HitInfo* hit, f32 tMax)
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

intern bool CheckHitLeafNode(KDTree* tree, KDLeaf* leaf, Ray* ray, Object** objHit, HitInfo* hit, f32 tMax)
{
    HitInfo hitClosest = {.tIntersect = INF};
    Object* objClosest = NULL;

    for (size_t ii = 0; ii < leaf->len; ii++) {
        HitInfo hitCur;
        Object* objCur = tree->objPtrs->at[leaf->objIndex + ii];

        if (Surface_HitAt(&objCur->surface, ray, RT_EPSILON, tMax, &hitCur)) {
            if (hitCur.tIntersect < hitClosest.tIntersect) {
                objClosest = objCur;
                hitClosest = hitCur;
            }
        }
    }

    if (objClosest != NULL) {
        *hit    = hitClosest;
        *objHit = objClosest;
        return true;
    }

    return false;
}

intern bool
CheckHitInternalNode(KDTree* tree, KDInternal* node, Ray* ray, Object** objHit, HitInfo* hit, Axis axis, f32 tMax)
{
    f32 split = node->split;

    KDNode* left  = &tree->nodes->at[node->leftIndex];
    KDNode* right = (KDNode*)node->right;

    if (unlikely(fabsf(ray->dir.elem[axis]) < RT_EPSILON)) {
        // ray parallel to plane, check the origin to see which side ray falls on
        if (ray->origin.elem[axis] >= split) {
            return CheckHitNextNode(tree, right, ray, objHit, hit, tMax);
        } else {
            return CheckHitNextNode(tree, left, ray, objHit, hit, tMax);
        }
    }

    f32 tIntersect = split * ray->cache.invDir.elem[axis] - ray->cache.originDivDir.elem[axis];
    f32 tMaxNew    = minf(tMax, tIntersect);

    if (tIntersect < RT_EPSILON) {
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
        KDNode* originSide;
        KDNode* oppositeSide;

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

bool KDTree_HitAt(KDTree* tree, Ray* ray, Object** objHit, HitInfo* hit)
{
    // then we traverse the tree and check for collisions
    KDNode* root = &tree->nodes->at[tree->rootIndex];
    return CheckHitNextNode(tree, root, ray, objHit, hit, INF);
}
