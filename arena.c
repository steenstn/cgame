#include "game.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct Arena {
    uint8_t* base;
    size_t size;
    size_t used;
} Arena;

void arena_init(Arena *arena, void* memory, size_t size) {
    arena->base = (uint8_t*)memory;
    arena->size = size;
    arena->used = 0;
}

void* arena_alloc(Arena *arena, size_t size) {
    size_t aligned_size = (size + 7) & ~7;

    if (arena->used + aligned_size > arena->size) {
        printf("Ran out of memory");
        exit(1);
    }

    void *result = arena->base + arena->used;
    arena->used += aligned_size;
    return result;
}

//void* arena_push(Arena *arena, RenderCommand *command) {
//}

void arena_clear(Arena *arena) {
    arena->used = 0;
}
