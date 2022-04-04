#include "kdtree.h"

#include <stdlib.h>

#include "gfx/utils.h"
#include "object/object.h"

// return the first index who's bounding box fully lies >= x
static size_t BinarySearchX(ObjectBB** pBoxes, size_t len, float x)
{
    ssize_t left  = 0;
    ssize_t right = len;
    ssize_t ret   = len;

    while (left <= right) {
        ssize_t cur = (left + right) / 2;
        if (pBoxes[cur]->box.pMin.x < x && pBoxes[cur]->box.pMax.x < x) {
            left = cur + 1;
        } else {
            ret   = cur;
            right = cur - 1;
        }
    }

    return (size_t)ret;
}

// return the first index who's bounding box fully lies >= y
static size_t BinarySearchY(ObjectBB** pBoxes, size_t len, float y)
{
    ssize_t left  = 0;
    ssize_t right = len;
    ssize_t ret   = len;

    while (left <= right) {
        size_t cur = (left + right) / 2;
        if (pBoxes[cur]->box.pMin.y < y && pBoxes[cur]->box.pMax.y < y) {
            left = cur + 1;
        } else {
            ret   = cur;
            right = cur - 1;
        }
    }

    return (size_t)ret;
}

// return the first index who's bounding box fully lies >= z
static size_t BinarySearchZ(ObjectBB** pBoxes, size_t len, float z)
{
    ssize_t left  = 0;
    ssize_t right = len;
    ssize_t ret   = len;

    while (left <= right) {
        size_t cur = (left + right) / 2;
        if (pBoxes[cur]->box.pMin.z < z && pBoxes[cur]->box.pMax.z < z) {
            left = cur + 1;
        } else {
            ret   = cur;
            right = cur - 1;
        }
    }

    return (size_t)ret;
}

static KDNode* KDTree_BuildX(ObjectBB* sort[], size_t len);
static KDNode* KDTree_BuildY(ObjectBB* sort[], size_t len);
static KDNode* KDTree_BuildZ(ObjectBB* sort[], size_t len);

static KDNode* KDTree_BuildX(ObjectBB* sort[], size_t len)
{
    // now we sort the boxes by their pMin.x values
    ObjectBB_SortX(sort, len);

    // for the partitioning to work, the dividing value can't intersect an object's bb (for objects that should fall to
    // the left). Otherwise it can't partition the objects
    // TODO: there might be a way to do this in log time since it's sorted, but maybe not, because we can't
    // sort by two values. We could sort by center point, but that still doesn't solve the problem
    // take the middle element's xMin as the initial splitX
    float splitX = sort[len / 2]->box.pMin.x;
    for (size_t ii = 0; ii < len; ii++) {
        // check for intersection
        if (within(splitX, sort[ii]->box.pMin.x, sort[ii]->box.pMax.x)) {
            // change splitX and reiterate the array
            splitX = sort[ii]->box.pMin.x;
            ii     = 0;
            continue;
        }
    }

    // split the array where all elems with index >= startRightHalf lie to the right of splitX
    // and all elems with index < startRightHalf lie to the left of splitX
    size_t startRightHalf = BinarySearchX(sort, len, splitX);

    if (startRightHalf == 0 || startRightHalf == len) {
        // all nodes lie to the right or to the left of splitX
        // we return a leaf node (Objects)
        KDNode* leaf = malloc(sizeof(*leaf) + len * sizeof(Object));
        if (leaf == NULL) {
            return NULL;
        }

        // initialize the node and copy in the objects
        leaf->type     = KD_LEAF;
        leaf->objs.len = len;
        for (size_t ii = 0; ii < len; ii++) {
            leaf->objs.elem[ii] = *sort[ii]->obj;
        }

        return leaf;
    } else {
        // some land on the right, some on the left, so we need to split again by Y
        // we return a parent node (xNode)
        KDNode* ltX = KDTree_BuildY(&sort[0], startRightHalf);
        if (ltX == NULL) {
            return NULL;
        }

        KDNode* gteqX = KDTree_BuildY(&sort[startRightHalf], len - startRightHalf);
        if (gteqX == NULL) {
            free(ltX);
            return NULL;
        }

        KDNode* parent = malloc(sizeof(*parent));
        if (parent == NULL) {
            free(ltX);
            free(gteqX);
            return NULL;
        }

        parent->type       = KD_PARENT;
        parent->node.split = splitX;
        parent->node.lt    = ltX;
        parent->node.gte   = gteqX;

        return parent;
    }
}

static KDNode* KDTree_BuildY(ObjectBB* sort[], size_t len)
{
    ObjectBB_SortY(sort, len);

    float splitY = sort[len / 2]->box.pMin.y;
    for (size_t ii = 0; ii < len; ii++) {
        if (within(splitY, sort[ii]->box.pMin.y, sort[ii]->box.pMax.y)) {
            splitY = sort[ii]->box.pMin.y;
            ii     = 0;
            continue;
        }
    }

    size_t startRightHalf = BinarySearchY(sort, len, splitY);

    if (startRightHalf == 0 || startRightHalf == len) {
        KDNode* leaf = malloc(sizeof(*leaf) + len * sizeof(Object));
        if (leaf == NULL) {
            return NULL;
        }

        leaf->type     = KD_LEAF;
        leaf->objs.len = len;
        for (size_t ii = 0; ii < len; ii++) {
            leaf->objs.elem[ii] = *sort[ii]->obj;
        }

        return leaf;
    } else {
        KDNode* ltY = KDTree_BuildZ(&sort[0], startRightHalf);
        if (ltY == NULL) {
            return NULL;
        }

        KDNode* gteqY = KDTree_BuildZ(&sort[startRightHalf], len - startRightHalf);
        if (gteqY == NULL) {
            free(ltY);
            return NULL;
        }

        KDNode* parent = malloc(sizeof(*parent));
        if (parent == NULL) {
            free(ltY);
            free(gteqY);
            return NULL;
        }

        parent->type       = KD_PARENT;
        parent->node.split = splitY;
        parent->node.lt    = ltY;
        parent->node.gte   = gteqY;

        return parent;
    }
}

static KDNode* KDTree_BuildZ(ObjectBB* sort[], size_t len)
{
    ObjectBB_SortZ(sort, len);

    float splitZ = sort[len / 2]->box.pMin.z;
    for (size_t ii = 0; ii < len; ii++) {
        if (within(splitZ, sort[ii]->box.pMin.z, sort[ii]->box.pMax.z)) {
            splitZ = sort[ii]->box.pMin.z;
            ii     = 0;
            continue;
        }
    }

    size_t startRightHalf = BinarySearchZ(sort, len, splitZ);

    if (startRightHalf == 0 || startRightHalf == len) {
        KDNode* leaf = malloc(sizeof(*leaf) + len * sizeof(Object));
        if (leaf == NULL) {
            return NULL;
        }

        leaf->type     = KD_LEAF;
        leaf->objs.len = len;
        for (size_t ii = 0; ii < len; ii++) {
            leaf->objs.elem[ii] = *sort[ii]->obj;
        }

        return leaf;
    } else {
        KDNode* ltZ = KDTree_BuildX(&sort[0], startRightHalf);
        if (ltZ == NULL) {
            return NULL;
        }

        KDNode* gteqZ = KDTree_BuildX(&sort[startRightHalf], len - startRightHalf);
        if (gteqZ == NULL) {
            free(ltZ);
            return NULL;
        }

        KDNode* parent = malloc(sizeof(*parent));
        if (parent == NULL) {
            free(ltZ);
            free(gteqZ);
            return NULL;
        }

        parent->type       = KD_PARENT;
        parent->node.split = splitZ;
        parent->node.lt    = ltZ;
        parent->node.gte   = gteqZ;

        return parent;
    }
}

// TODO: optimize this for cache locality and minimize the number of allocations
// seems pretty difficult to preallocate all the memory contiguously because of the recursive nature of these functions
// getting them to share the buffer correctly is a challenge
//
// returns: A KD tree with X -> Y -> Z traversal for parents (if type is KD_PARENT)
// could also be a leaf node (KD_LEAF)
KDNode* KDTree_Build(ObjectBB* boxes, size_t len)
{
    // we need a buffer of pointers to the objects BB's
    ObjectBB** pBoxes = calloc(len, sizeof(ObjectBB*));
    if (pBoxes == NULL) {
        return NULL;
    }

    for (size_t ii = 0; ii < len; ii++) {
        pBoxes[ii] = &boxes[ii];
    }

    return KDTree_BuildX(pBoxes, len);
}
