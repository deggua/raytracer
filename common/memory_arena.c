#include "memory_arena.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define popcount                __builtin_popcount
#define alignto(val, alignMask) ((val) + ((-(val)) & (alignMask)))

// NOTE: alignment must be a power of 2
MemoryArena* MemoryArena_New(size_t totalMemory, size_t alignment)
{
    assert(popcount(alignment) == 1);

    MemoryArena* arena = malloc(sizeof(*arena) + alignto(totalMemory, alignment - 1));
    arena->capacity    = alignto(totalMemory, alignment - 1);
    arena->used        = 0;
    arena->next        = arena->alloc;
    arena->alignMask   = alignment - 1;

    return arena;
}

void MemoryArena_Delete(MemoryArena* arena)
{
    free(arena);
}

void* MemoryArena_Malloc(MemoryArena* arena, size_t size)
{
    size_t memConsumed = alignto(size, arena->alignMask);
    if (arena->capacity - arena->used < memConsumed) {
        printf("Ran out of memory in arena\n");
        return NULL;
    }

    void* alloc = arena->next;
    arena->next += memConsumed;
    arena->used += memConsumed;

    return alloc;
}
