#include <stdbool.h>
#include <stdint.h>

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
} GameState;

typedef struct GameAPI {
    GameState *(*init)();
    bool (*step)(GameState* state);
} GameAPI;

GameAPI* get_game_api();

