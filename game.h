#include <stdbool.h>
#include <stdint.h>

#include "engine.c"

typedef struct Thing {
    float x;
    float y;
    float oldx;
    float oldy;
    int image_index;
} Thing;

typedef struct GameState {
    Thing* things;
    char* level;
    int screenWidth;
    int screenHeight;
    int32_t mouseX;
    int32_t mouseY;
    int r,g,b;
} GameState;

typedef struct PlatformAPI {
    wRenderer* renderer;
} PlatformAPI;

typedef struct GameAPI {
    GameState *(*init)();
    bool (*step)(GameState* state, wRenderer* renderer);
} GameAPI;

GameAPI* get_game_api();

