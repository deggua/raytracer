#include <stdbool.h>
#include <stddef.h>

#if !defined(VECTOR_INCLUDED)
    #define VECTOR_INCLUDED
    #if !defined(CONCAT2)
        #define CONCAT2_(a, b) a##_##b
        #define CONCAT2(a, b)  CONCAT2_(a, b)
    #endif

    #define Vector_GrowTo(T) CONCAT2(Vector_GrowTo, T)

    /* --------- PUBLIC API ---------- */

    // Include Parameters:
    //  The type of the elements the vector should contain:
    //      #define TEMPLATE_TYPE                   T
    //
    //  The name to use to reference the type specific macros:
    //      #define TEMPLATE_NAME                   CustomName
    //      default: TEMPLATE_TYPE
    //
    //  The initial length the vector should allocate to:
    //      Default: 16 elements
    //      #define TEMPLATE_INIT_CAPACITY          16
    //
    //  The growth function the vector should use when it needs to expand:
    //      Default: (3 * old_size + 1) / 2
    //      #define TEMPLATE_GROW(old_size)         (3 * old_size + 1) / 2
    //
    //  An allocator function (obeying ISO C's malloc/calloc semantics):
    //      Default: malloc
    //      #define TEMPLATE_MALLOC(bytes)          malloc(bytes)
    //
    //  A reallocator function (obeying ISO C's realloc semantics):
    //      Default: realloc
    //      #define TEMPLATE_REALLOC(ptr, bytes)    realloc(ptr, bytes)
    //
    //  A free function (obeying ISO C's free semantics):
    //      Default: free
    //      #define TEMPLATE_FREE(ptr)              free(ptr)

    // Functions:
    //  The vector type
    #define Vector(T) CONCAT2(Vector, T)

    // Create a new vector
    //  Vector(T)* Vector_New(T)()
    #define Vector_New(T) CONCAT2(Vector_New, T)

    // Delete a vector
    //  void Vector_Delete(T)(Vector(T)* vec)
    #define Vector_Delete(T) CONCAT2(Vector_Delete, T)

    // Ensure a vector is preallocated to some length
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Reserve(T)(Vector(T)* vec, size_t length)
    #define Vector_Reserve(T) CONCAT2(Vector_Reserve, T)

    // Copy an element to the end of the vector
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Push(T)(Vector(T)* vec, T* element)
    #define Vector_Push(T) CONCAT2(Vector_Push, T)

    // Copy multiple elements to the end of the vector in order
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_PushMany(T)(Vector(T)* vec, T* element, size_t length)
    #define Vector_PushMany(T) CONCAT2(Vector_PushMany, T)

    // Copy an element to a particular index in the vector
    //  The index specified must be a valid index in the vector, or just past the last element in the vector
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Insert(T)(Vector(T)* vec, size_t index, T* element)
    #define Vector_Insert(T) CONCAT2(Vector_Insert, T)

    // Copy multiple elements into the vector, starting at a particular index in the vector
    //  The index specified must be a valid index in the vector, or just past the last element in the vector
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Insert(T)(Vector(T)* vec, size_t index, T* element, size_t length)
    #define Vector_InsertMany(T) CONCAT2(Vector_InsertMany, T)

    // Remove an element from the vector
    //  The index specified must be a valid index in the vector
    //  void Vector_Remove(T)(Vector(T)* vec, size_t index)
    #define Vector_Remove(T) CONCAT2(Vector_Remove, T)

    // Remove a range of elements from the vector
    //  The all indexes in the range specified must be a valid index in the vector
    //  The range removed is [start, stop) (not inclusive of the final index)
    //  void Vector_RemoveRange(T)(Vector(T)* vec, size_t start, size_t stop)
    #define Vector_RemoveRange(T) CONCAT2(Vector_RemoveRange, T)

    // Shrink the vector's allocated memory to fit the number of elements in the vector
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Shrink(T)(Vector(T)* vec)
    #define Vector_Shrink(T) CONCAT2(Vector_Shrink, T)

    // Empties a vector and frees the underlying buffer
    //  Returns true if operation succeeded, false otherwise
    //  bool Vector_Clear(T)(Vector(T)* vec)
    #define Vector_Clear(T) CONCAT2(Vector_Clear, T)

    // Pops an element off the end of the vector
    //  bool Vector_Pop(T)(Vector(T)* vec, T* dest)
    #define Vector_Pop(T) CONCAT2(Vector_Pop, T)

    // Pops multiple elements off the end of the vector
    // size_t Vector_PopMany(T)(Vector(T)* vec, T[] dest, size_t length)
    #define Vector_PopMany(T) CONCAT2(Vector_PopMany, T)

    // Allocates space at the end of the vector without putting any data at that element,
    // and returns the index that element was placed at. Returns -1 if the vector failed to realloc
    #define Vector_PushEmpty(T) CONCAT2(Vector_PushEmpty, T)

    /* --------- END PUBLIC API ---------- */
#endif

#if !defined(TEMPLATE_TYPE)
    #error "Vector requires a type specialization"
#endif

#if !defined(TEMPLATE_NAME)
    #define TEMPLATE_NAME TEMPLATE_TYPE
#endif

#if !defined(TEMPLATE_INIT_CAPACITY)
    #define TEMPLATE_INIT_CAPACITY 16
#endif

#if !defined(TEMPLATE_GROW)
    #define TEMPLATE_GROW(old_size) ((3 * old_size + 1) / 2)
#endif

#if !defined(TEMPLATE_MALLOC)
    #if !defined(DEFAULT_ALLOCATOR)
        #define DEFAULT_ALLOCATOR
    #endif

    #define TEMPLATE_MALLOC(bytes) calloc(1, bytes)
#endif

#if !defined(TEMPLATE_REALLOC)
    #if !defined(DEFAULT_ALLOCATOR)
        #warning "Non-default malloc used with default realloc"
    #endif

    #define TEMPLATE_REALLOC realloc
#endif

#if !defined(TEMPLATE_FREE)
    #if !defined(DEFAULT_ALLOCATOR)
        #warning "Non-default malloc used with default free"
    #endif

    #define TEMPLATE_FREE free
#endif

#if defined(DEFAULT_ALLOCATOR)
    #include <stdlib.h>
#endif

// useful macros internally
#define T                    TEMPLATE_TYPE
#define Tn                   TEMPLATE_NAME
#define vector_init_capacity TEMPLATE_INIT_CAPACITY
#define vector_grow          TEMPLATE_GROW
#define vector_malloc        TEMPLATE_MALLOC
#define vector_realloc       TEMPLATE_REALLOC
#define vector_free          TEMPLATE_FREE
#define vector_max(a, b)     ((a) > (b) ? (a) : (b))

// data types

typedef struct Vector(Tn)
{
    T*     at;
    size_t length;
    size_t capacity;
}
Vector(Tn);

// functions

static inline Vector(Tn)* Vector_New(Tn)(void)
{
    Vector(Tn)* vec = vector_malloc(sizeof(Vector(Tn)));

    if (vec == NULL) {
        return NULL;
    }

    T* buffer = vector_malloc(sizeof(T) * vector_init_capacity);

    if (buffer == NULL) {
        vector_free(vec);
        return NULL;
    }

    vec->length   = 0;
    vec->capacity = vector_init_capacity;
    vec->at       = buffer;

    return vec;
}

static inline void Vector_Delete(Tn)(Vector(Tn)* vec)
{
    vector_free(vec->at);
    vector_free(vec);
}

static inline bool Vector_GrowTo(Tn)(Vector(Tn)* vec, size_t length)
{
    T* newBuffer = realloc(vec->at, sizeof(T) * length);

    if (newBuffer == NULL) {
        return false;
    }

    vec->at       = newBuffer;
    vec->capacity = length;

    return true;
}

static inline bool Vector_Reserve(Tn)(Vector(Tn)* vec, size_t length)
{
    if (vec->capacity >= length) {
        return true;
    }

    return Vector_GrowTo(Tn)(vec, length);
}

static inline bool Vector_PushMany(Tn)(Vector(Tn)* vec, const T elems[], size_t length)
{
    // check if we need to grow the vector
    if (vec->capacity < vec->length + length) {
        size_t newCapacity;

        if (vec->capacity == 0) {
            newCapacity = vector_max(vec->length + length, vector_init_capacity);
        } else {
            newCapacity = vector_max(vec->length + length, vector_grow(vec->capacity));
        }

        if (!Vector_GrowTo(Tn)(vec, newCapacity)) {
            return false;
        }
    }

    for (size_t ii = 0; ii < length; ++ii) {
        vec->at[vec->length + ii] = (T)elems[ii];
    }

    vec->length += length;

    return true;
}

static inline bool Vector_Push(Tn)(Vector(Tn)* vec, const T* elem)
{
    return Vector_PushMany(Tn)(vec, elem, 1);
}

// TODO: improve API
static inline ssize_t Vector_PushEmpty(Tn)(Vector(Tn)* vec)
{
    if (Vector_Reserve(Tn)(vec, vec->length + 1)) {
        vec->length += 1;
        return vec->length - 1;
    } else {
        return -1;
    }
}

static inline bool Vector_InsertMany(Tn)(Vector(Tn)* vec, size_t index, const T elems[], size_t length)
{
    if (length == 0) {
        // nop
        return true;
    } else if (index == vec->length) {
        // equivalent to a push many
        return Vector_PushMany(Tn)(vec, elems, length);
    } else if (index > vec->length) {
        // can't insert past the end of the vector
        return false;
    }

    // check if we need to grow the vector
    if (vec->capacity < vec->length + length) {
        size_t newCapacity;

        if (vec->capacity == 0) {
            newCapacity = vector_max(vec->length + length, vector_init_capacity);
        } else {
            newCapacity = vector_max(vec->length + length, vector_grow(vec->capacity));
        }

        if (!Vector_GrowTo(Tn)(vec, newCapacity)) {
            return false;
        }
    }

    // copy elements (starting at the end) to their new position at the end of the vector
    for (size_t ii = vec->length + length - 1; ii > index + length - 1; --ii) {
        vec->at[ii] = vec->at[ii - length];
    }

    // copy elements from elems to their new position starting at the end of their section in the vector
    for (size_t ii = index + length - 1; ii > index; --ii) {
        vec->at[ii] = (T)elems[ii - index];
    }

    // copy first element in elems to index
    vec->at[index] = (T)elems[0];

    vec->length += length;

    return true;
}

static inline bool Vector_Insert(Tn)(Vector(Tn)* vec, size_t index, const T* elem)
{
    return Vector_InsertMany(Tn)(vec, index, elem, 1);
}

static inline bool Vector_Shrink(Tn)(Vector(Tn)* vec)
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
        vector_free(vec->at);
        vec->at = NULL;
    } else {
        size_t shrunkCapacity = sizeof(T) * vec->length;
        T*     shrunkBuffer   = vector_realloc(vec->at, shrunkCapacity);

        if (shrunkBuffer == NULL) {
            return false;
        }

        vec->at = shrunkBuffer;
    }

    vec->capacity = vec->length;

    return true;
}

static inline bool Vector_Clear(Tn)(Vector(Tn)* vec)
{
    vector_free(vec->at);
    vec->at = NULL;

    vec->length   = 0;
    vec->capacity = 0;

    return true;
}

static inline void Vector_RemoveRange(Tn)(Vector(Tn)* vec, size_t start, size_t stop)
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

// Remove an element from the vector
//  The index specified must be a valid index in the vector
//  void Vector_Remove(T)(Vector(T)* vec, size_t index)
static inline void Vector_Remove(Tn)(Vector(Tn)* vec, size_t index)
{
    Vector_RemoveRange(Tn)(vec, index, index + 1);
}

// cleanup macros
#undef T
#undef Tn
#undef vector_init_capacity
#undef vector_grow
#undef vector_malloc
#undef vector_realloc
#undef vector_free
#undef vector_max

#undef DEFAULT_ALLOCATOR

#undef TEMPLATE_TYPE
#undef TEMPLATE_NAME
#undef TEMPLATE_INIT_CAPACITY
#undef TEMPLATE_GROW
#undef TEMPLATE_MALLOC
#undef TEMPLATE_REALLOC
#undef TEMPLATE_FREE
