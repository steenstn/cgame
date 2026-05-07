
#include "game.h"
#include "game_engine.c"
#include <assert.h>

int main() {

    void* mem = malloc(1024);
    Arena arena;
    arena_init(&arena, mem, 500);

    Queue queue = {};
    queue.entries = arena_alloc(&arena, 20);
    queue.capacity = 3;
    queue_push(&queue, 1);
    queue_push(&queue, 2);
    queue_push(&queue, 3);
    queue_push(&queue, 4);
    int res1 = queue_pop(&queue);

    assert(res1 == 1);

return 0;
}
