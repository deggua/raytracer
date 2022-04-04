#include <stdbool.h>
#include <stddef.h>

#if !defined(_VECTOR_INCLUDED)
#    define _VECTOR_INCLUDED
#    if !defined(_CONCAT2)
#        define _CONCAT2_(a, b) a##_##b
#        define _CONCAT2(a, b)  _CONCAT2_(a, b)
#    endif

#    define _Vector_GrowTo(T) _CONCAT2(_Vector_GrowTo, T)

/* --------- PUBLIC API ---------- */

// Include Parameters:
//  The type of the elements the vector should contain:
//      #define REQ_PARAM_type                 T
//
//  The initial length the vector should allocate to:
//      Default: 16 elements
//      #define OPT_PARAM_init_capacity         16
//
//  The growth function the vector should use when it needs to expand:
//      Default: 2 * old_size
//      #define OPT_PARAM_grow(old_size)        (2 * old_size)
//
//  An allocator function (obeying ISO C's malloc/calloc semantics):
//      Default: malloc
//      #define OPT_PARAM_malloc(bytes)        malloc(bytes)
//
//  A reallocator function (obeying ISO C's realloc semantics):
//      Default: realloc
//      #define OPT_PARAM_realloc(ptr, bytes)  realloc(ptr, bytes)
//
//  A free function (obeying ISO C's free semantics):
//      Default: free
//      #define OPT_PARAM_free(ptr)            free(ptr)

// Functions:
//  The vector type
#    define Vector(T) _CONCAT2(Vector, T)

// Create a new vector
//  Vector(T)* Vector_New(T)()
#    define Vector_New(T) _CONCAT2(Vector_New, T)

// Delete a vector
//  void Vector_Delete(T)(Vector(T)* vec)
#    define Vector_Delete(T) _CONCAT2(Vector_Delete, T)

// Ensure a vector is preallocated to some length
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Reserve(T)(Vector(T)* vec, size_t length)
#    define Vector_Reserve(T) _CONCAT2(Vector_Reserve, T)

// Copy an element to the end of the vector
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Push(T)(Vector(T)* vec, const T* element)
#    define Vector_Push(T) _CONCAT2(Vector_Push, T)

// Copy multiple elements to the end of the vector in order
//  Returns true if operation succeeded, false otherwise
//  bool Vector_PushMany(T)(Vector(T)* vec, const T* element, size_t length)
#    define Vector_PushMany(T) _CONCAT2(Vector_PushMany, T)

// Copy an element to a particular index in the vector
//  The index specified must be a valid index in the vector, or just past the last element in the vector
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Insert(T)(Vector(T)* vec, size_t index, const T* element)
#    define Vector_Insert(T) _CONCAT2(Vector_Insert, T)

// Copy multiple elements into the vector, starting at a particular index in the vector
//  The index specified must be a valid index in the vector, or just past the last element in the vector
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Insert(T)(Vector(T)* vec, size_t index, const T* element, size_t length)
#    define Vector_InsertMany(T) _CONCAT2(Vector_InsertMany, T)

// Remove an element from the vector
//  The index specified must be a valid index in the vector
//  void Vector_Remove(T)(Vector(T)* vec, size_t index)
#    define Vector_Remove(T) _CONCAT2(Vector_Remove, T)

// Remove a range of elements from the vector
//  The all indexes in the range specified must be a valid index in the vector
//  The range removed is [start, stop) (not inclusive of the final index)
//  void Vector_RemoveRange(T)(Vector(T)* vec, size_t start, size_t stop)
#    define Vector_RemoveRange(T) _CONCAT2(Vector_RemoveRange, T)

// Shrink the vector's allocated memory to fit the number of elements in the vector
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Shrink(T)(Vector(T)* vec)
#    define Vector_Shrink(T) _CONCAT2(Vector_Shrink, T)

// Empties a vector and frees the underlying buffer
//  Returns true if operation succeeded, false otherwise
//  bool Vector_Clear(T)(Vector(T)* vec)
#    define Vector_Clear(T) _CONCAT2(Vector_Clear, T)

// Pops an element off the end of the vector
//  bool Vector_Pop(T)(Vector(T)* vec, T* dest)
#    define Vector_Pop(T) _CONCAT2(Vector_Pop, T)

// Pops multiple elements off the end of the vector
// size_t Vector_PopMany(T)(Vector(T)* vec, T[] dest, size_t length)
#    define Vector_PopMany(T) _CONCAT2(Vector_PopMany, T)

/* --------- END PUBLIC API ---------- */
#endif

#if !defined(REQ_PARAM_type)
#    error "Vector requires a type specialization"
#endif

#if !defined(OPT_PARAM_init_capacity)
#    define OPT_PARAM_init_capacity 16
#endif

#if !defined(OPT_PARAM_grow)
#    define OPT_PARAM_grow(old_size) (2 * old_size)
#endif

#if !defined(OPT_PARAM_malloc)
#    if !defined(_DEFAULT_ALLOCATOR)
#        define _DEFAULT_ALLOCATOR
#    endif

#    define OPT_PARAM_malloc(bytes) calloc(1, bytes)
#endif

#if !defined(OPT_PARAM_realloc)
#    if !defined(_DEFAULT_ALLOCATOR)
#        warning "Non-default malloc used with default realloc"
#    endif

#    define OPT_PARAM_realloc realloc
#endif

#if !defined(OPT_PARAM_free)
#    if !defined(_DEFAULT_ALLOCATOR)
#        warning "Non-default malloc used with default free"
#    endif

#    define OPT_PARAM_free free
#endif

#if defined(_DEFAULT_ALLOCATOR)
#    include <stdlib.h>
#endif

// useful macros internally

#define T                    REQ_PARAM_type
#define vector_init_capacity OPT_PARAM_init_capacity
#define vector_grow          OPT_PARAM_grow
#define malloc               OPT_PARAM_malloc
#define realloc              OPT_PARAM_realloc
#define free                 OPT_PARAM_free

#ifndef max
#    define DEFD_MAX
#    define max(a, b) ((a) > (b) ? (a) : (b))
#endif

// data types

typedef struct Vector(T)
{
    T*     at;
    size_t length;
    size_t capacity;
}
Vector(T);

// functions

static Vector(T) * Vector_New(T)(void)
{
    Vector(T)* vec = malloc(sizeof(Vector(T)));
    if (vec == NULL) {
        return NULL;
    }

    T* buffer = malloc(sizeof(T) * vector_init_capacity);
    if (buffer == NULL) {
        free(vec);
        return NULL;
    }

    vec->length   = 0;
    vec->capacity = vector_init_capacity;
    vec->at       = buffer;

    return vec;
}

static void Vector_Delete(T)(Vector(T) * vec)
{
    free(vec->at);
    free(vec);
}

static bool _Vector_GrowTo(T)(Vector(T) * vec, size_t length)
{
    T* newBuffer = realloc(vec->at, sizeof(T) * length);
    if (newBuffer == NULL) {
        return false;
    }

    vec->at       = newBuffer;
    vec->capacity = length;

    return true;
}

static bool Vector_Reserve(T)(Vector(T) * vec, size_t length)
{
    if (vec->capacity >= length) {
        return true;
    }

    return _Vector_GrowTo(T)(vec, length);
}

static bool Vector_PushMany(T)(Vector(T) * vec, const T elems[], size_t length)
{
    // check if we need to grow the vector
    if (vec->capacity < vec->length + length) {
        size_t newCapacity;
        if (vec->capacity == 0) {
            newCapacity = max(vec->length + length, vector_init_capacity);
        } else {
            newCapacity = max(vec->length + length, vector_grow(vec->capacity));
        }

        if (!_Vector_GrowTo(T)(vec, newCapacity)) {
            return false;
        }
    }

    for (size_t ii = 0; ii < length; ++ii) {
        vec->at[vec->length + ii] = elems[ii];
    }
    vec->length += length;

    return true;
}

static bool Vector_Push(T)(Vector(T) * vec, const T* elem)
{
    return Vector_PushMany(T)(vec, elem, 1);
}

static bool Vector_InsertMany(T)(Vector(T) * vec, size_t index, const T elems[], size_t length)
{
    if (length == 0) {
        // nop
        return true;
    } else if (index == vec->length) {
        // equivalent to a push many
        return Vector_PushMany(T)(vec, elems, length);
    } else if (index > vec->length) {
        // can't insert past the end of the vector
        return false;
    }

    // check if we need to grow the vector
    if (vec->capacity < vec->length + length) {
        size_t newCapacity;
        if (vec->capacity == 0) {
            newCapacity = max(vec->length + length, vector_init_capacity);
        } else {
            newCapacity = max(vec->length + length, vector_grow(vec->capacity));
        }

        if (!_Vector_GrowTo(T)(vec, newCapacity)) {
            return false;
        }
    }

    // copy elements (starting at the end) to their new position at the end of the vector
    for (size_t ii = vec->length + length - 1; ii > index + length - 1; --ii) {
        vec->at[ii] = vec->at[ii - length];
    }

    // copy elements from elems to their new position starting at the end of their section in the vector
    for (size_t ii = index + length - 1; ii > index; --ii) {
        vec->at[ii] = elems[ii - index];
    }

    // copy first element in elems to index
    vec->at[index] = elems[0];

    vec->length += length;

    return true;
}

static bool Vector_Insert(T)(Vector(T) * vec, size_t index, const T* elem)
{
    return Vector_InsertMany(T)(vec, index, elem, 1);
}

static bool Vector_Shrink(T)(Vector(T) * vec)
{
    // nop if it's already shrunk to size
    if (vec->capacity == vec->length) {
        return true;
    }

    // needs special handling for resizes to 0, realloc basically acts as a free in that case
    // which implies that the old memory is invalidated, the problem is realloc returns NULL,
    // so we can't distinguish whether we did a realloc(ptr, 0) or realloc failed unless we check
    // before we call realloc
    if (vec->length == 0) {
        free(vec->at);
        vec->at = NULL;
    } else {
        size_t shrunkCapacity = sizeof(T) * vec->length;
        T*     shrunkBuffer   = realloc(vec->at, shrunkCapacity);
        if (shrunkBuffer == NULL) {
            return false;
        }
        vec->at = shrunkBuffer;
    }

    vec->capacity = vec->length;

    return true;
}

static bool Vector_Clear(T)(Vector(T) * vec)
{
    free(vec->at);
    vec->at = NULL;

    vec->length   = 0;
    vec->capacity = 0;

    return true;
}

static void Vector_RemoveRange(T)(Vector(T) * vec, size_t start, size_t stop)
{
    if (start > vec->length - 1 || stop > vec->length || stop - start == 0) {
        // nop
        return;
    }

    for (size_t old_ii = stop, new_ii = start; old_ii < vec->length; ++old_ii, ++new_ii) {
        vec->at[new_ii] = vec->at[old_ii];
    }

    vec->length -= stop - start;
}

static void Vector_Remove(T)(Vector(T) * vec, size_t index)
{
    Vector_RemoveRange(T)(vec, index, index + 1);
}

// cleanup macros
#undef T
#undef vector_init_capacity
#undef vector_grow
#undef malloc
#undef realloc
#undef free
#ifdef DEFD_MAX
#    undef max
#    undef DEFD_MAX
#endif

#undef _DEFAULT_ALLOCATOR

#undef REQ_PARAM_type
#undef OPT_PARAM_init_capacity
#undef OPT_PARAM_grow
#undef OPT_PARAM_malloc
#undef OPT_PARAM_realloc
#undef OPT_PARAM_free
