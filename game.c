#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game_engine.h"
#include "game.h"

enum Flags {
    FLAG_PLAYER_CONTROLLED = 1,
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
    }
    state->things[0].flags = FLAG_PLAYER_CONTROLLED;

    state->screenWidth = 1600;
    state->screenHeight = 1200;
    state->mouseX = 0;
    state->mouseY = 0;
    state->levelWidth = 60;
    state->levelHeight = 30;
    state->tileSize = 100;
    state->r = 100;
    state->level = arena_alloc(&state->permanent_arena, 60*30);

    state->keys_down = arena_alloc(&state->permanent_arena, _NUM_KEY_CODES);
    state->keys_down[KEY_UP] = 'w'-93;
    state->keys_down[KEY_LEFT] = 'a'-93;
    state->keys_down[KEY_DOWN] = 's'-93;
    state->keys_down[KEY_RIGHT] = 'd'-93;

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


static bool update_and_render(GameState* state, const uint8_t* key_states) {
        /*for(int i = 0; i < _NUM_KEY_CODES; i++) {
            printf("lol: %d", key_states[i]);
            state->keys_down[i] = key_states[state->keys_down[i]];
            printf("state->keys_down[%d]: %d\n",i, state->keys_down[i]);
        }*/
        for(int i = 0; i < 512; i++) {
            if (key_states[i]) {

            printf("Also: %d\n", (int)'a');
            printf("%d: %d\n", i, key_states[i]);
            printf("lol: %d\n",state->keys_down[KEY_LEFT]);
            }
        }

        

        float speed = 5.0;
                if (key_states[SCANCODE_A]) {
                    state->viewportX-=speed;
                }
                if (key_states[SCANCODE_S]) {
                    state->viewportY+=speed;
                }
                if (key_states[SCANCODE_D]) {
                    state->viewportX+=speed;
                }
                if (key_states[SCANCODE_W]) {
                    state->viewportY-=speed;
                }
        MouseState* mouse = &state->mouse_state;
        for(int i = 0; i < 3; i++) {
            Thing* t = &state->things[i];
            if(state->mouse_state.left_button_click && aabb_collision(mouse->x, mouse->y, 1, 1, -state->viewportX+t->x, -state->viewportY+t->y, 200, 200)) {
                t->flags = 1 - t->flags;
            }
            if(t->flags == FLAG_PLAYER_CONTROLLED) {
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
    memset(state->output_buffer, 0, 1600*1200*4);

    for(int y = 0; y < state->levelHeight; y++) {
        for(int x = 0; x < state->levelWidth; x++) {
            if(state->level[ARRAY_INDEX(x, y, 60)] == '1') {
                drawRect(state, -state->viewportX+x*100, -state->viewportY+y*100, 100, 100, 0xffffffff);
            }
        }

    }

    for(int i = 0; i < 3; i++) {
        Thing t = state->things[i];
        uint32_t color = t.flags ? 0xff00ff00 : 0x4f4f4f;
        drawRect(state, -state->viewportX+t.x,-state->viewportY+t.y,200,200, color);
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


