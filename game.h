#include <stdbool.h>
#include <stdint.h>

#include "engine.c"
#include "arena.c"
#define MAX_THINGS 100

typedef struct Thing {
    float x;
    float y;
    float oldx;
    float oldy;
    int image_index;
} Thing;

typedef struct GameMemory {
    void* permanent_storage;
    size_t permanent_storage_size;
} GameMemory;


typedef struct GameState {
    Arena permanent_arena;
    Thing* things;
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
    void* output_buffer;
} GameState;


typedef struct PlatformAPI {
    wRenderer* renderer;
} PlatformAPI;

typedef struct GameAPI {
    GameState *(*init)(GameMemory* gameMemory);
    bool (*step)(GameState* state, wRenderer* renderer);
} GameAPI;

GameAPI* get_game_api();

