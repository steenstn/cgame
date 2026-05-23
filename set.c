#include "game.h"
typedef struct DictEntry {
    vec2_i key;
    vec2_i value;
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

static vec2_i dict_get_value(Dict* dict, vec2_i key) {
    for(int i = 0; i < dict->size; i++) {
        vec2_i entry_key = dict->entries[i].key; 
        if(entry_key.x == key.x && entry_key.y == key.y) {
            return dict->entries[i].value;
        }
    }
    printf("Could not find key (%d, %d) in dict\n", key.x, key.y);
    return (vec2_i){-1, -1};
}

static bool dict_has_value(Dict* dict, vec2_i value) {
    for(int i = 0; i < dict->size; i++) {
        vec2_i dict_value = dict->entries[i].value; 
        if(dict_value.x == value.x && dict_value.y == value.y) {
            return true;
        }
    }
    return false;
}

static bool dict_has_key(Dict* dict, vec2_i key) {
    for(int i = 0; i < dict->size; i++) {
        vec2_i entry_key = dict->entries[i].key; 
        if(entry_key.x == key.x && entry_key.y == key.y) {
            return true;
        }
    }
    return false;
}

static bool dict_set_value(Dict* dict, vec2_i key, vec2_i value) {
    for(int i = 0; i < dict->size; i++) {
        DictEntry* current = &dict->entries[i];
        if (current->key.x == key.x && current->key.y == key.y) {
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
