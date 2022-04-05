#pragma once

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define ARENA_MAX_ALIGNMENT 16

typedef struct {
    uint8_t* next;
    size_t   capacity;
    size_t   used;
    size_t   alignMask;
    alignas(ARENA_MAX_ALIGNMENT) uint8_t alloc[];
} MemoryArena;

MemoryArena* MemoryArena_New(size_t totalMemory, size_t alignment);
void         MemoryArena_Delete(MemoryArena* arena);
void*        MemoryArena_Malloc(MemoryArena* arena, size_t size);
