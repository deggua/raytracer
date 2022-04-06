#include "kdtree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/memory_arena.h"
#include "gfx/utils.h"
#include "object/object.h"

typedef struct {
    size_t index;
    size_t diff;
    float  value;
} Split;

static size_t FirstSplitIndex(ObjectBB* sorted[], size_t len, float split, VecAxis axis)
{
    ssize_t left  = 0;
    ssize_t right = len;
    ssize_t ret   = len;

    while (left <= right) {
        ssize_t cur = (left + right) / 2;
        if (sorted[cur]->box.pMin.e[axis] < split && sorted[cur]->box.pMax.e[axis] < split) {
            left = cur + 1;
        } else {
            ret   = cur;
            right = cur - 1;
        }
    }

    return (size_t)ret;
}

static KDNode* SplitAxis(MemoryArena* arena, ObjectBB* sort[], size_t len);

static KDNode* BuildParentNode(MemoryArena* arena, ObjectBB* sorted[], size_t len, const Split* split, VecAxis axis)
{
    KDNode* lt = SplitAxis(arena, &sorted[0], split->index);
    if (lt == NULL) {
        goto error_lt;
    }

    KDNode* parent = MemoryArena_Malloc(arena, sizeof(*parent));
    if (parent == NULL) {
        goto error_parent;
    }

    KDNode* gteq = SplitAxis(arena, &sorted[split->index], len - split->index);
    if (gteq == NULL) {
        goto error_gteq;
    }

    parent->type       = (KDNodeType)axis;
    parent->node.split = split->value;
    parent->node.lt    = lt;
    parent->node.gteq  = gteq;

    const char* axisName;
    switch (axis) {
        case VEC_X:
            axisName = "x";
            break;
        case VEC_Y:
            axisName = "y";
            break;
        case VEC_Z:
            axisName = "z";
            break;
    };
    printf("Created partition in the %s-axis at %s = %.2f\n", axisName, axisName, split->value);

    return parent;

    // TODO: should I bother figuring out how to do deallocations in the arena?
error_parent:
    // free(gteq);
error_gteq:
    // free(lt);
error_lt:
    return NULL;
}

static KDNode* BuildLeafNode(MemoryArena* arena, ObjectBB* sorted[], size_t len)
{
    KDNode* leaf = MemoryArena_Malloc(arena, sizeof(*leaf) + len * sizeof(Object));
    if (leaf == NULL) {
        return NULL;
    }

    // initialize the node and copy in the objects
    leaf->type     = KD_LEAF;
    leaf->objs.len = len;
    for (size_t ii = 0; ii < len; ii++) {
        leaf->objs.elem[ii] = *sorted[ii]->obj;
    }

    printf("Created leaf node with %zu objects\n", len);
    return leaf;
}

static size_t SplitDiff(size_t index, size_t len)
{
    size_t elemLeft  = index;
    size_t elemRight = len - index;

    size_t splitDiff;
    if (elemLeft > elemRight) {
        splitDiff = elemLeft - elemRight;
    } else {
        splitDiff = elemRight - elemLeft;
    }

    return splitDiff;
}

// TODO: O(n^2), optimize if possible
static Split BestSplit(ObjectBB* sorted[], size_t len, VecAxis axis)
{
    const size_t optimalSplitDiff = len % 2;

    Split best = {
        .index = 0,
        .diff  = len,
        .value = sorted[0]->box.pMin.e[axis],
    };

    for (size_t ii = 1; ii < len; ii++) {
        float split = sorted[ii]->box.pMin.e[axis];

        // splits that intersect a bb are invalid
        for (size_t xx = 0; xx < len && xx != ii; xx++) {
            if (within(split, sorted[xx]->box.pMin.e[axis], sorted[xx]->box.pMax.e[axis])) {
                goto split_intersects;
            }
        }

        // get the true split index (split index can be a bit funny if pMin values equal)
        // TODO: there's got to be a better way to do this
        size_t splitIndex = FirstSplitIndex(sorted, len, split, axis);

        size_t splitDiff = SplitDiff(splitIndex, len);
        if (splitDiff < best.diff) {
            best.diff  = splitDiff;
            best.value = split;
            best.index = splitIndex;
        }

        // we can terminate early here because you can't do any better
        if (splitDiff == optimalSplitDiff) {
            return best;
        }

split_intersects:
        continue;
    }

    return best;
}

// determines which axis is the best to split, and where to do said split.
// TODO: is there a better way to do this than checking each axis?
static KDNode* SplitAxis(MemoryArena* arena, ObjectBB* sort[], size_t len)
{
    const size_t optimalSplitDiff = len % 2;

    if (len <= 25) {
        // don't split if it's less than some threshold, probably depends on the number of objects in the scene what
        // the optimal value is, might be computable but 25 seems good from testing
        return BuildLeafNode(arena, sort, len);
    }

    ObjectBB_Sort(sort, len, VEC_X);
    Split x = BestSplit(sort, len, VEC_X);
    if (x.diff == optimalSplitDiff) {
        return BuildParentNode(arena, sort, len, &x, VEC_X);
    }

    ObjectBB_Sort(sort, len, VEC_Y);
    Split y = BestSplit(sort, len, VEC_Y);
    if (y.diff == optimalSplitDiff) {
        return BuildParentNode(arena, sort, len, &y, VEC_Y);
    }

    ObjectBB_Sort(sort, len, VEC_Z);
    Split z = BestSplit(sort, len, VEC_Z);
    if (z.diff == optimalSplitDiff) {
        return BuildParentNode(arena, sort, len, &z, VEC_Z);
    }

    // TODO: remove the redundant sorts by keeping a copy of the best sort so far, probably faster for large numbers of
    // objects, just requires more memory and more allocations, but I assume the cost of a sort is larger than the cost
    // of a malloc (might not be, needs to be profiled)
    if (x.diff == len && y.diff == len && z.diff == len) {
        return BuildLeafNode(arena, sort, len);
    } else if (x.diff <= y.diff && x.diff <= z.diff) {
        ObjectBB_Sort(sort, len, VEC_X);
        return BuildParentNode(arena, sort, len, &x, VEC_X);
    } else if (y.diff <= x.diff && y.diff <= z.diff) {
        ObjectBB_Sort(sort, len, VEC_Y);
        return BuildParentNode(arena, sort, len, &y, VEC_Y);
    } else if (z.diff <= x.diff && z.diff <= y.diff) {
        ObjectBB_Sort(sort, len, VEC_Z);
        return BuildParentNode(arena, sort, len, &z, VEC_Z);
    } else {
        // impossible, but let's print and exit just to be safe
        // TODO: try to enable the following and see if code gen is still correct
        // __builtin_unreachable();
        printf("Reached invalid condition in kdtree.c:SplitAxis\n");
        exit(1);
    }
}

// TODO: I think we could preallocate 2 * len buffer of ObjectBB*, so split axis can use them
KDNode* KDTree_Build(ObjectBB* boxes, size_t len)
{
    // we need a buffer of pointers to the objects BB's
    ObjectBB** pBoxes = calloc(len, sizeof(ObjectBB*));
    if (pBoxes == NULL) {
        return NULL;
    }
    // TODO: free the pBoxes buffer before we return KDNode
    // TODO: return a more generic object so we can free the KDTree (probably a KDNode* root and a MemoryArena* arena)

    for (size_t ii = 0; ii < len; ii++) {
        pBoxes[ii] = &boxes[ii];
    }

    // max amount of memory (I'm pretty sure) is (2*N-1) * sizeof(KDNode) + len * sizeof(Object)
    MemoryArena* arena = MemoryArena_New(2 * ((2 * len - 1) * sizeof(KDNode) + len * sizeof(Object)), 16);

    return SplitAxis(arena, pBoxes, len);
}
