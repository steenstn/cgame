#include <stdbool.h>
#include <stdint.h>

#include "arena.c"
#define MAX_THINGS 100


typedef struct GameMemory {
    bool is_initialized;
    void* permanent_storage;
    size_t permanent_storage_size;
} GameMemory;


typedef struct GameState {
    Arena permanent_arena;
    uint8_t* level;
    int levelWidth;
    int levelHeight;
    int tileSize;
    int screenWidth;
    int screenHeight;
    int32_t mouseX;
    int32_t mouseY;
    int viewportX;
    int viewportY;
    int r,g,b;
    uint32_t* output_buffer;
} GameState;


typedef struct GameAPI {
    GameState *(*init)(GameMemory* gameMemory);
    bool (*step)(GameState* state);
} GameAPI;

GameAPI* get_game_api();

