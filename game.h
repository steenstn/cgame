#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "game_engine.h"
#include "arena.c"
#define MAX_THINGS 500
#define SCREEN_WIDTH 1400
#define SCREEN_HEIGHT 1050

typedef struct PlatformAPI {
    char* (*get_stuff)();
    void* (*read_whole_file)(char* path);
} PlatformAPI;

typedef struct GameMemory {
    bool is_initialized;
    void* permanent_storage;
    size_t permanent_storage_size;
    PlatformAPI platform_api;

} GameMemory;

typedef struct Thing {
    float x,y, old_x, old_y;
    float vx, vy;
    int width, height;
    int projectile_counter;
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
    int viewportX;
    int viewportY;
    void* image;
    void* image2;
    void* image3;

    uint8_t* keys_down;
    MouseState mouse_state;
    uint32_t* output_buffer;
} GameState;


typedef struct GameAPI {
    GameState *(*init)(GameMemory* gameMemory);
    bool (*update_and_render)(GameState* state, const uint8_t* key_states);
} GameAPI;

GameAPI* get_game_api();


void* platform_read_whole_file(char* path);

#endif 
