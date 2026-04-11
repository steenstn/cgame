#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

enum Flags {
    IS_ACTIVE = 1<<0,
    FLAG_PLAYER_CONTROLLED = 1<<1,
    FLAG_PROJECTILE = 1<<2,
};


static GameState *init(GameMemory* gameMemory) {

    GameState* state = (GameState*)gameMemory->permanent_storage;
    if (gameMemory->is_initialized) {
        return state;
    }
    u8* arena_base = (u8*)gameMemory->permanent_storage + sizeof(GameState);

    arena_init(&state->permanent_arena, arena_base, gameMemory->permanent_storage_size - sizeof(GameState));

    state->things = arena_alloc(&state->permanent_arena, MAX_THINGS);
    for(int i = 0; i < 3; i++) {
        state->things[i].x = i*400;
        state->things[i].width = 20;
        state->things[i].height = 20;
        state->things[i].flags = IS_ACTIVE;
    }
    state->things[0].flags = FLAG_PLAYER_CONTROLLED | IS_ACTIVE;
    state->things[0].x = 400;
    state->things[0].y = 400;


    state->screenWidth = SCREEN_WIDTH;
    state->screenHeight = SCREEN_HEIGHT;
    state->levelWidth = 60;
    state->levelHeight = 30;
    state->tileSize = 100;
    state->r = 100;
    state->level = arena_alloc(&state->permanent_arena, 60*30);

    state->keys_down = arena_alloc(&state->permanent_arena, _NUM_KEY_CODES);
    state->keys_down[KEY_UP] = SCANCODE_W;
    state->keys_down[KEY_LEFT] = SCANCODE_A;
    state->keys_down[KEY_DOWN] = SCANCODE_S;
    state->keys_down[KEY_RIGHT] = SCANCODE_D;
    state->keys_down[KEY_SHIFT] = SCANCODE_LSHIFT;

    state->output_buffer = arena_alloc(&state->permanent_arena, state->screenHeight*state->screenWidth*4);

    u8* level = state->level;
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

static bool aabb_collision(float x1, float y1, float w1, float h1,
                          float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 &&
           x1 + w1 > x2 &&
           y1 < y2 + h2 &&
           y1 + h1 > y2;
}
// is this correct?
static int clamp(int value, int min, int max) {
    return value > min ? value < max ? value : min : max;
}

static void drawPixel(GameState* state, int x, int y, uint32_t color) {
    state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
}

static void drawRect(GameState* state, int _x, int _y, int width, int height, uint32_t color) {
        int x_end = _x+width;
        int y_end = _y+height;
        for(int y = _y; y < y_end; y++) {
            if(y <0 || y >= state->screenHeight) {
                continue;
            }
            for(int x = _x; x < x_end; x++) {
                if (x<0 || x>=state->screenWidth) {
                    continue;
                }
                state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
            }
        }
}


static bool update_and_render(GameState* state, const u8* key_states) {
        /*for(int i = 0; i < _NUM_KEY_CODES; i++) {
            printf("lol: %d", key_states[i]);
            state->keys_down[i] = key_states[state->keys_down[i]];
            printf("state->keys_down[%d]: %d\n",i, state->keys_down[i]);
        }*/

        for(int i = 0; i < 512; i++) {
            if (key_states[i]) {

            printf("Also: %d\n", (int)'a');
            printf("%d: %d\n", i, key_states[i]);
            }
        }


        float speed = 5.0;

        MouseState* mouse = &state->mouse_state;

        state->viewportX = state->things[0].x+mouse->x*0.9-SCREEN_WIDTH;
        state->viewportY = state->things[0].y+mouse->y*0.9-SCREEN_HEIGHT;

        if (key_states[SCANCODE_LSHIFT]) {
            speed = 8;
        }

        for(int i = 0; i < 3; i++) {
            Thing* t = &state->things[i];
            if(state->mouse_state.left_button_click && aabb_collision(mouse->x, mouse->y, 1, 1, -state->viewportX+t->x, -state->viewportY+t->y, 200, 200)) {
                t->flags = flags_flip(t->flags, FLAG_PLAYER_CONTROLLED);
            }
            if(flags_contains(t->flags, FLAG_PLAYER_CONTROLLED)) {
                if (key_states[SCANCODE_A]) {
                    t->x-=speed;
                }
                if (key_states[SCANCODE_S]) {
                    t->y+=speed;
                }
                if (key_states[SCANCODE_D]) {
                    t->x+=speed;
                }
                if (key_states[SCANCODE_W]) {
                    t->y-=speed;
                }
            }
        }

    //---------- Render 
    memset(state->output_buffer, 0, SCREEN_WIDTH*SCREEN_HEIGHT*4);

    // TODO Do not draw all tiles, only inside viewport
    for(int y = 0; y < state->levelHeight; y++) {
        for(int x = 0; x < state->levelWidth; x++) {
            if(state->level[ARRAY_INDEX(x, y, 60)] == '1') {
                drawRect(state, -state->viewportX+x*100, -state->viewportY+y*100, 100, 100, 0xffffffff);
            }
        }

    }

    for(int i = 0; i < 3; i++) {
        Thing t = state->things[i];
        uint32_t color = flags_contains(t.flags, FLAG_PLAYER_CONTROLLED) ? 0xff00ff00 : 0x4f4f4f;
        drawRect(state, -state->viewportX+t.x,-state->viewportY+t.y,t.width,t.height, color);
    }

    return true;
}


static GameAPI api = {
    .init = init,
    .update_and_render = update_and_render
};

GameAPI* get_game_api() {
    return &api;
}


