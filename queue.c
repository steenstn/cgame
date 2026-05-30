#include "game.h"

static Queue queue_init(Arena* arena, int capacity) {
    Queue queue = {
        .head = 0,
        .tail = 0,
        .entries = arena_alloc(arena, capacity * sizeof(int)),
        .capacity = capacity,
        .size = 0
    };
    return queue;
}

void queue_push(Queue* queue, int value) {
    if (queue->size >= queue->capacity) {
        return;
    }
    queue->entries[queue->tail] = value;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;
}

int queue_pop(Queue* queue) {
    if (queue->size <= 0) {
        return 0;
    }
    int value = queue->entries[queue->head];
    queue->head = (queue->head + 1) % (queue->capacity);
    queue->size--;
    return value;
}

static QueueVec2 queue_vec2_init(Arena* arena, int capacity) {
    QueueVec2 queue = {
        .head = 0,
        .tail = 0,
        .entries = arena_alloc(arena, capacity * sizeof(vec2_i)),
        .capacity = capacity,
        .size = 0
    };
    return queue;
}

void queue_vec2_push(QueueVec2* queue, vec2_i value) {
    if (queue->size >= queue->capacity) {
        return;
    }
    queue->entries[queue->tail] = value;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;
}

vec2_i queue_vec2_pop(QueueVec2* queue) {
    if (queue->size <= 0) {
        return (vec2_i){};
    }
    vec2_i value = queue->entries[queue->head];
    queue->head = (queue->head + 1) % (queue->capacity);
    queue->size--;
    return value;
}
