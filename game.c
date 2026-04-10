#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"

static GameState *init(GameMemory* gameMemory) {

    GameState* state = (GameState*)gameMemory->permanent_storage;
    uint8_t* arena_base = (uint8_t*)gameMemory->permanent_storage + sizeof(GameState);

    arena_init(&state->permanent_arena, arena_base, gameMemory->permanent_storage_size - sizeof(GameState));

    //arena_init(state.permanent_arena, gameMemory, gameMemory->permanent_storage_size);

    //state->things = arena_alloc(state->permanent_arena, sizeof(Thing)*2);
    state->screenWidth = 1600;
    state->screenHeight = 1200;
    state->mouseX = 0;
    state->mouseY = 0;
    state->levelWidth = 60;
    state->levelHeight = 30;
    state->tileSize = 100;
    state->level = arena_alloc(&state->permanent_arena, 60*30);

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

static bool step(GameState* state, wRenderer* renderer) {
    state->r = 200;
    state->g = 250;
    state->b = 10;
    uint8_t* level = state->level;
    int level_width = state->levelWidth;
    int level_height = state->levelHeight;
    int tile_size = state->tileSize;

    printf("%d\n", state->viewportX);
    for(int i = 0; i < level_width*level_height;i++) {
        int drawing_x = (i%level_width)*tile_size;
        int drawing_y = (i/level_width)*tile_size;
        if(level[i] == '.') {
            wSetRenderDrawColor(renderer, 100, 100, 100, 255);
        } else {
            wSetRenderDrawColor(renderer, state->r, state->g, state->b, 255);
        }
        wFillRect(renderer, -state->viewportX+drawing_x, -state->viewportY+drawing_y, tile_size, tile_size);

    }
    return true;
}

static GameAPI api = {
    .init = init,
    .step = step
};

GameAPI* get_game_api() {
    return &api;
}


