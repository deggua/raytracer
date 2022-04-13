#include "kdtree.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/cext.h"
#include "gfx/utils.h"
#include "object/object.h"

#define LEAF_MIN_LOAD 50
const float TraversalCost = 1.0f;
const float IntersectCost = 6.0f;

// TODO: refine traversal code to also check whether the plane intersection ray(t).e[axis] is within the bounds
// of the current voxel's region (have to pass + split the box as you move down the tree), imagine a ray
// passing through box, and hitting the inf plane outside of the current voxels box, this gives an early stopping
// condition where you only need to check the side the ray is on in the intersect case
// TODO: benchmark code with and without pointer indirection to object array (store Object copies vs copies of ptrs to
// Objects)

// TODO: track avg intersection tests / ray to determine what kind of improvements modification to the tree structure
// provide
// TODO: consider passing the tMin/tMax through the traversal code as it's refined so that HitAt can determine
// whether it needs to consider certain intersections

// TODO: consider adding max depth to tree
// TODO: consider adding weight to empty voxel (b_e)
// TODO: consider allowing bad splits up to some threshold (apparently bad splits may yield good splits later)

typedef enum
{
    KD_INTERNAL_X = VEC_X,
    KD_INTERNAL_Y = VEC_Y,
    KD_INTERNAL_Z = VEC_Z,
    KD_LEAF       = 3,
} KDNodeType;

typedef struct KDBB {
    BoundingBox   box;
    const Object* obj;
} KDBB;

typedef KDBB*   KDBBPtr;
typedef Object* ObjectPtr;

#define TEMPLATE_TYPE ObjectPtr
#include "common/vector.h"

#define TEMPLATE_TYPE KDBBPtr
#include "common/vector.h"

#define TEMPLATE_TYPE KDBB
#include "common/vector.h"

typedef struct {
    struct {
        KDNodeType   : 2;
        uint32_t len : 30;
    };
    uint32_t objIndex;
} KDLeaf;
static_assert(sizeof(KDLeaf) == 8, "sizeof(KDLeaf) != 8");

typedef struct {
    struct {
        KDNodeType         : 2;
        uint32_t leftIndex : 30;
    };
    float split;
} KDInternal;
static_assert(sizeof(KDInternal) == 8, "sizeof(KDInternal) != 8");

typedef union KDNode {
    struct {
        KDNodeType type : 2;
        uint32_t        : 30;
    };
    KDInternal inode;
    KDLeaf     leaf;
} KDNode;
static_assert(sizeof(KDNode) == 8, "sizeof(KDNode) != 8");

#define TEMPLATE_TYPE KDNode
#include "common/vector.h"

typedef struct KDTree {
    Vector(KDNode) * nodes;
    Vector(ObjectPtr) * objPtrs;
    BoundingBox worldBox;
    ssize_t     rootIndex;
} KDTree;

static ssize_t BuildNode(KDTree* tree, const Vector(KDBBPtr) * vec, BoundingBox container);

static ssize_t BuildParentNode(
    KDTree* tree,
    const Vector(KDBBPtr) * leftVec,
    BoundingBox leftContainer,
    const Vector(KDBBPtr) * rightVec,
    BoundingBox rightContainer,
    float       partition,
    VecAxis     axis)
{
    ssize_t parentIndex = Vector_PushEmpty(KDNode)(tree->nodes);
    if (parentIndex < 0) {
        goto error_parent;
    }
    printf("Created parent node at index %zd\n", parentIndex);

    // construct right node after parent, causes the next node to be at the index immediately after
    // parent's index (ie if you have the KDNode* to parent, parent + 1 == right child)
    ssize_t rightNode = BuildNode(tree, rightVec, rightContainer);
    if (rightNode < 0) {
        goto error_gteq;
    }
    printf("Created right child node at index %zd\n", rightNode);

    ssize_t leftNode = BuildNode(tree, leftVec, leftContainer);
    if (leftNode < 0) {
        goto error_lt;
    }
    printf("Created left child node at index %zd\n", leftNode);

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
    ssize_t nodeIndex = Vector_PushEmpty(KDNode)(tree->nodes);
    if (nodeIndex < 0) {
        return -1;
    }

    KDNode* node   = &tree->nodes->at[nodeIndex];
    node->type     = KD_LEAF;
    node->leaf.len = vec->length;

    ssize_t firstObjIndex = tree->objPtrs->length;
    for (size_t ii = 0; ii < vec->length; ii++) {
        // TODO: check if this returns true for error handling
        Vector_Push(ObjectPtr)(tree->objPtrs, (Object**)&vec->at[ii]->obj);
    }

    node->leaf.objIndex = firstObjIndex;

    printf("Created leaf node at index %zd with %u objects\n", nodeIndex, node->leaf.len);

    return nodeIndex;
}

static float SurfaceArea(BoundingBox box)
{
    const float xDim = box.max.x - box.min.x;
    const float yDim = box.max.y - box.min.y;
    const float zDim = box.max.z - box.min.z;

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
        for (size_t axis = 0; axis < 3; axis++) {
            if (kdbbs[ii].box.min.e[axis] < box.min.e[axis]) {
                box.min.e[axis] = kdbbs[ii].box.min.e[axis];
            }

            if (kdbbs[ii].box.max.e[axis] > box.max.e[axis]) {
                box.max.e[axis] = kdbbs[ii].box.max.e[axis];
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

static float ComputeSplitSAH(const Vector(KDBBPtr) * vec, float split, VecAxis axis, BoundingBox parent)
{
    const float parentSA = SurfaceArea(parent);

    KDBB** kdbbs = vec->at;

    BoundingBoxPair boxPair    = SplitBox(parent, split, axis);
    size_t          leftPrims  = 0;
    size_t          rightPrims = 0;

    for (size_t ii = 0; ii < vec->length; ii++) {
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

static ssize_t BuildNode(KDTree* tree, const Vector(KDBBPtr) * vec, BoundingBox container)
{
    if (vec->length <= LEAF_MIN_LOAD) {
        return BuildLeafNode(tree, vec);
    }

    float   bestSplit = 0.0f;
    VecAxis bestAxis  = VEC_X;
    float   bestSAH   = INF;

    // determine best split location via SAH
    // TODO: use buckets, this is infeasible for large scenes
    const size_t numBuckets = 64;

    for (VecAxis axis = VEC_X; axis < VEC_LAST; axis++) {
        float stride = (container.max.e[axis] - container.min.e[axis]) / numBuckets;
        for (float bucket = container.min.e[axis] + stride; bucket <= container.max.e[axis] - stride;
             bucket += stride) {
            float split = bucket;
            float SAH   = ComputeSplitSAH(vec, split, axis, container);
            if (SAH < bestSAH) {
                bestSAH   = SAH;
                bestAxis  = axis;
                bestSplit = split;
            }
        }
    }

    float parentSAH = vec->length * IntersectCost;
    if (parentSAH <= bestSAH) {
        // cost of best split outweighs just putting everything in a leaf
        return BuildLeafNode(tree, vec);
    } else {
        // cost of best split is better than cost of total, split them up
        // TODO: add failure checking
        Vector(KDBBPtr)* leftVec  = Vector_New(KDBBPtr)();
        Vector(KDBBPtr)* rightVec = Vector_New(KDBBPtr)();

        Vector_Reserve(KDBBPtr)(leftVec, vec->length);
        Vector_Reserve(KDBBPtr)(rightVec, vec->length);

        for (size_t ii = 0; ii < vec->length; ii++) {
            const BoundingBox* box = &vec->at[ii]->box;
            if (box->max.e[bestAxis] < bestSplit) {
                Vector_Push(KDBBPtr)(leftVec, &vec->at[ii]);
            } else if (box->min.e[bestAxis] > bestSplit) {
                Vector_Push(KDBBPtr)(rightVec, &vec->at[ii]);
            } else {
                Vector_Push(KDBBPtr)(leftVec, &vec->at[ii]);
                Vector_Push(KDBBPtr)(rightVec, &vec->at[ii]);
            }
        }

        BoundingBoxPair pair = SplitBox(container, bestSplit, bestAxis);
        ssize_t nodeIndex    = BuildParentNode(tree, leftVec, pair.left, rightVec, pair.right, bestSplit, bestAxis);

        Vector_Delete(KDBBPtr)(leftVec);
        Vector_Delete(KDBBPtr)(rightVec);

        return nodeIndex;
    }

    return -1;
}

static ssize_t BuildKDTree(KDTree* tree, const Vector(KDBB) * boxes)
{
    // we need a buffer of pointers to the objects BB's
    Vector(KDBBPtr)* vec = Vector_New(KDBBPtr)();
    Vector_Reserve(KDBBPtr)(vec, boxes->length);

    for (size_t ii = 0; ii < boxes->length; ii++) {
        KDBB* ptr = &boxes->at[ii];
        Vector_Push(KDBBPtr)(vec, &ptr);
    }

    return BuildNode(tree, vec, tree->worldBox);
}

static Vector(KDBB) * CreateKDBBs(const Object objs[], size_t len)
{
    Vector(KDBB)* vec = Vector_New(KDBB)();
    Vector_Reserve(KDBB)(vec, len);

    for (size_t ii = 0; ii < len; ii++) {
        KDBB temp;
        temp.obj = &objs[ii];
        Surface_BoundedBy(&objs[ii].surface, &temp.box);
        Vector_Push(KDBB)(vec, &temp);
    }

    return vec;
}

KDTree* KDTree_New(const Object objs[], size_t len)
{
    KDTree* tree = calloc(1, sizeof(KDTree));
    if (tree == NULL) {
        goto error_KDTree;
    }

    tree->nodes = Vector_New(KDNode)();
    if (tree->nodes == NULL) {
        goto error_NodeVector;
    }

    tree->objPtrs = Vector_New(ObjectPtr)();
    if (tree->objPtrs == NULL) {
        goto error_ObjVector;
    }

    Vector(KDBB)* boxes = CreateKDBBs(objs, len);
    if (boxes == NULL) {
        goto error_KDBBs;
    }

    tree->worldBox  = BoxBoundingAll(boxes);
    tree->rootIndex = BuildKDTree(tree, boxes);
    if (tree->rootIndex < 0) {
        goto error_Root;
    }

    Vector_Delete(KDBB)(boxes);
    return tree;

error_Root:
    Vector_Delete(KDBB)(boxes);
error_KDBBs:
    Vector_Delete(ObjectPtr)(tree->objPtrs);
error_ObjVector:
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

static bool CheckHitLeafNode(const KDTree* tree, const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit);

static bool CheckHitInternalNode(
    const KDTree*     tree,
    const KDInternal* node,
    const Ray*        ray,
    Object**          objHit,
    HitInfo*          hit,
    VecAxis           axis,
    BoundingBox       bounds);

static bool CheckHitNextNode(
    const KDTree* tree,
    const KDNode* node,
    const Ray*    ray,
    Object**      objHit,
    HitInfo*      hit,
    BoundingBox   bounds)
{
    switch (node->type) {
        case KD_INTERNAL_X:
        case KD_INTERNAL_Y:
        case KD_INTERNAL_Z: {
            return CheckHitInternalNode(tree, &node->inode, ray, objHit, hit, (VecAxis)node->type, bounds);
        }

        case KD_LEAF: {
            return CheckHitLeafNode(tree, &node->leaf, ray, objHit, hit);
        } break;
    };

    return false;
}

static bool CheckHitLeafNode(const KDTree* tree, const KDLeaf* leaf, const Ray* ray, Object** objHit, HitInfo* hit)
{
    HitInfo       hitClosest = {.tIntersect = INF};
    const Object* objClosest = NULL;

    for (size_t ii = 0; ii < leaf->len; ii++) {
        HitInfo       hitCur;
        const Object* objCur = tree->objPtrs->at[leaf->objIndex + ii];

        if (Surface_HitAt(&objCur->surface, ray, 0.001f, INF, &hitCur)) {
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

static const char* AxisName(VecAxis axis)
{
    switch (axis) {
        case VEC_X:
            return "x";
        case VEC_Y:
            return "y";
        case VEC_Z:
            return "z";
        default:
            return "?";
    }
}

// TODO: we probably want to pass KDNode instead of KDInteral since we have to type pun to get to the right node,
// which might be UB
static bool CheckHitInternalNode(
    const KDTree*     tree,
    const KDInternal* node,
    const Ray*        ray,
    Object**          objHit,
    HitInfo*          hit,
    VecAxis           axis,
    BoundingBox       bounds)
{
    const float           split            = node->split;
    const float           epsilonParallel  = 0.0001f;
    const float           epsilonIntersect = 0.001f;
    const BoundingBoxPair childBounds      = SplitBox(bounds, split, axis);

    const KDNode* left  = &tree->nodes->at[node->leftIndex];
    const KDNode* right = &((KDNode*)node)[1];

    if (unlikely(fabsf(ray->dir.e[axis]) < epsilonParallel)) {
        // ray parallel to plane, check the origin to see which side ray falls on
        // TODO: is this correct when the origin is in the plane?
        if (ray->origin.e[axis] >= split) {
            return CheckHitNextNode(tree, right, ray, objHit, hit, childBounds.right);
        } else {
            return CheckHitNextNode(tree, left, ray, objHit, hit, childBounds.left);
        }
    }

    float tIntersect = split * ray->cache.invDir.e[axis] - ray->cache.originDivDir.e[axis];
    if (tIntersect < epsilonIntersect) {
        // ray doesn't intersect plane at valid t, check the origin to see which side ray falls on
        if (likely(ray->origin.e[axis] > split)) {
            return CheckHitNextNode(tree, right, ray, objHit, hit, childBounds.right);
        } else if (likely(ray->origin.e[axis] < split)) {
            return CheckHitNextNode(tree, left, ray, objHit, hit, childBounds.left);
        } else {
            // ray intersects the plane at the origin, eval the ray later and see what side it's on
            float rayAt = Ray_At(ray, 1.0f).e[axis];
            if (rayAt >= split) {
                return CheckHitNextNode(tree, right, ray, objHit, hit, childBounds.right);
            } else {
                return CheckHitNextNode(tree, left, ray, objHit, hit, childBounds.left);
            }
        }
    } else {
        // intersects with the plane, which means it could intersect objects on either side. we can check the side the
        // origin is on first and then if it doesn't hit anything on that side check the opposite since the ray will
        // hit objects on the origin side before objects on the opposite side
        const KDNode* originSide;
        const KDNode* oppositeSide;

        if (ray->origin.e[axis] >= split) {
            originSide   = right;
            oppositeSide = left;
            if (CheckHitNextNode(tree, originSide, ray, objHit, hit, childBounds.right)
                && hit->position.e[axis] >= split) {
                // the object hit needs to lie on the right side of the plane
                return true;
            } else {
                Point3  planeIntersect = Ray_At(ray, tIntersect);
                VecAxis otherAxis1     = (axis + 1) % VEC_LAST;
                VecAxis otherAxis2     = (axis + 2) % VEC_LAST;

                if ((planeIntersect.e[otherAxis1] <= bounds.max.e[otherAxis1])
                    && (planeIntersect.e[otherAxis1] >= bounds.min.e[otherAxis1])
                    && (planeIntersect.e[otherAxis2] <= bounds.max.e[otherAxis2])
                    && (planeIntersect.e[otherAxis2] >= bounds.min.e[otherAxis2])) {
                    // only check the opposite side if the ray-plane intersection lies inside the bounds of the current
                    // node's voxel
                    return CheckHitNextNode(tree, oppositeSide, ray, objHit, hit, childBounds.left);
                } else {
                    return false;
                }
            }
        } else {
            originSide   = left;
            oppositeSide = right;
            if (CheckHitNextNode(tree, originSide, ray, objHit, hit, childBounds.left)
                && hit->position.e[axis] < split) {
                // the object hit needs to lie on the left side of the plane
                return true;
            } else {
                Point3  planeIntersect = Ray_At(ray, tIntersect);
                VecAxis otherAxis1     = (axis + 1) % VEC_LAST;
                VecAxis otherAxis2     = (axis + 2) % VEC_LAST;

                if ((planeIntersect.e[otherAxis1] <= bounds.max.e[otherAxis1])
                    && (planeIntersect.e[otherAxis1] >= bounds.min.e[otherAxis1])
                    && (planeIntersect.e[otherAxis2] <= bounds.max.e[otherAxis2])
                    && (planeIntersect.e[otherAxis2] >= bounds.min.e[otherAxis2])) {
                    // only check the opposite side if the ray-plane intersection lies inside the bounds of the current
                    // node's voxel
                    return CheckHitNextNode(tree, oppositeSide, ray, objHit, hit, childBounds.right);
                } else {
                    return false;
                }
            }
        }
    }
}

bool KDTree_HitAt(const KDTree* tree, const Ray* ray, Object** objHit, HitInfo* hit)
{
    // then we traverse the tree and check for collisions
    KDNode* root = &tree->nodes->at[tree->rootIndex];
    return CheckHitNextNode(tree, root, ray, objHit, hit, tree->worldBox);
}
