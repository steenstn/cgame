#include "game.h"
typedef struct DictEntry {
    int key;
    int value;
} DictEntry;

typedef struct Dict {
    DictEntry* entries;
    int capacity;
    int size;
} Dict;


static Dict dict_init(Arena* arena, int capacity) {
    Dict dict = {
        .entries = arena_alloc(arena, capacity * sizeof(DictEntry)),
        .capacity = capacity,
        .size = 0
    };
    return dict;
}

static int dict_get_value(Dict* dict, int key) {
    for(int i = 0; i < dict->size; i++) {
        if(dict->entries[i].key == key) {
            return dict->entries[i].value;
        }
    }
    printf("Could not find key %d in dict\n", key);
    return 0;
}

static bool dict_has_value(Dict* dict, int value) {
    for(int i = 0; i < dict->size; i++) {
        if(dict->entries[i].value == value) {
            return true;
        }
    }
    return false;
}

static bool dict_has_key(Dict* dict, int key) {
    for(int i = 0; i < dict->size; i++) {
        if(dict->entries[i].key == key) {
            return true;
        }
    }
    return false;
}

static bool dict_set_value(Dict* dict, int key, int value) {
    for(int i = 0; i < dict->size; i++) {
        DictEntry* current = &dict->entries[i];
        if (current->key == key) {
            current->value = value;
            return true;
        }
    }

    if(dict->size >= dict->capacity) {
        printf("Dictionary is at capacity, cannot add value\n");
        return false;
    }
    
    dict->entries[dict->size].key = key;
    dict->entries[dict->size].value = value;

    dict->size++;
    return true;
    
}
