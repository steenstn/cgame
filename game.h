#include <stdbool.h>
#include <stdint.h>

#include "arena.c"
#define MAX_THINGS 100
#define SCREEN_WIDTH 1376
#define SCREEN_HEIGHT 1080

typedef struct GameMemory {
    bool is_initialized;
    void* permanent_storage;
    size_t permanent_storage_size;
} GameMemory;

typedef struct Thing {
    float x,y;
    uint64_t flags;
} Thing;

typedef struct MouseState {
    int x,y;
    bool left_button_down;
    bool right_button_down;
    bool left_button_click;
    bool right_button_click;
} MouseState;

 
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

    uint8_t* keys_down;
    MouseState mouse_state;
    uint32_t* output_buffer;
} GameState;


typedef struct GameAPI {
    GameState *(*init)(GameMemory* gameMemory);
    bool (*update_and_render)(GameState* state, const uint8_t* key_states);
} GameAPI;

GameAPI* get_game_api();

