#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

#define ARRAY_INDEX(x, y, width) (x + width*y)

static GameState *init(GameMemory* gameMemory) {

    GameState* state = (GameState*)gameMemory->permanent_storage;
    if (gameMemory->is_initialized) {
        return state;
    }
    uint8_t* arena_base = (uint8_t*)gameMemory->permanent_storage + sizeof(GameState);

    arena_init(&state->permanent_arena, arena_base, gameMemory->permanent_storage_size - sizeof(GameState));

    state->screenWidth = 1600;
    state->screenHeight = 1200;
    state->mouseX = 0;
    state->mouseY = 0;
    state->levelWidth = 60;
    state->levelHeight = 30;
    state->tileSize = 100;
    state->level = arena_alloc(&state->permanent_arena, 60*30);
    state->output_buffer = arena_alloc(&state->permanent_arena, state->screenHeight*state->screenWidth*4);

    uint8_t* level = state->level;
    int level_width = state->levelWidth;
    int level_height = state->levelHeight;

    for(int i = 0; i < level_width*level_height; i++) {
        level[i] = '.';
        if (i%level_width == 0 || i <= level_width || i > (level_width*level_height)-level_width || ((i+1)%(level_width))==0 || i%83==0) {
            level[i] = '1';
        }
        if (i > 0 && level[(i-1)] == '1') {
            if( rand() % 10 > 3) {
                level[i] = '1';
            }

        }
    }
    return state;
}


static void drawRect(GameState* state, int _x, int _y, int width, int height, uint32_t color) {
        int x_end = _x+width;
        int y_end = _y+height;
        for(int y = _y; y < y_end; y++) {
            for(int x = _x; x < x_end; x++) {
                state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
            }
        }
}

static bool step(GameState* state) {

    memset(state->output_buffer, 0, 1600*1200*4);

    drawRect(state, 100,100,200,200, 0xff0000ff);
    drawRect(state, 200,500,100,200, 0xff0000ff);
/*
    for(int i = 0; i < 1600*1200*4;i+=4) {
        state->output_buffer[i] = 0;
        state->output_buffer[i+1] = 200;
        state->output_buffer[i+2] = 255;
        state->output_buffer[i+3] = 0;
    }
    */
    return true;
}


static GameAPI api = {
    .init = init,
    .step = step
};

GameAPI* get_game_api() {
    return &api;
}


