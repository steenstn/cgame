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

// is this correct?
static int clamp(int value, int min, int max) {
    return value > min ? value < max ? value : min : max;
}

static void drawRect(GameState* state, int _x, int _y, int width, int height, uint32_t color) {
        int x_end = _x+width;
        int y_end = _y+height;
        for(int y = _y; y < y_end; y++) {
            if(y <0 || y > state->screenHeight) {
                continue;
            }
            for(int x = _x; x < x_end; x++) {
                if (x<0 || x>state->screenWidth) {
                    continue;
                }
                state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
            }
        }
}


static bool update_and_render(GameState* state) {

    state->mouseX= (state->mouseX + 2) % state->screenWidth;
    memset(state->output_buffer, 0, 1600*1200*4);

    for(int y = 0; y < 30; y++) {
        for(int x = 0; x < 60; x++) {
            if(state->level[ARRAY_INDEX(x, y, 60)] == '1') {

                drawRect(state, x*100, y*100, 100, 100, 0xffffffff);
            }
        }

    }

    drawRect(state, state->mouseX,100,200,200, 0x110000ff);
    drawRect(state, 100,150,20,200, 0x110000ff);
    return true;
}


static GameAPI api = {
    .init = init,
    .update_and_render = update_and_render
};

GameAPI* get_game_api() {
    return &api;
}


