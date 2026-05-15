
#include "game.h"
#include "game_engine.c"
#include <assert.h>


void test_dict() {

    void* mem = malloc(1024);
    Arena arena;
    arena_init(&arena, mem, 500);

    Dict dict = dict_init(&arena, 3);
    dict_set_value(&dict, 20, 10);
    dict_set_value(&dict, 20, 12);
    int res = dict_get_value(&dict, 20);
    assert(res == 12);
    int res2 = dict_get_value(&dict, 99990);
    assert(res2 == 0);
}

void test_queue() {
    void* mem = malloc(1024);
    Arena arena;
    arena_init(&arena, mem, 500);

    Queue q = queue_init(&arena, 10);
    queue_push(&q, (vec2){2,3});
    queue_push(&q, (vec2){2,3});
    queue_push(&q, (vec2){2,3});
    queue_push(&q, (vec2){2,3});
    queue_push(&q, (vec2){2,3});
    printf("q %d\n", q.size);
}

int main() {

    //test_dict();
    test_queue();

    return 0;
}
