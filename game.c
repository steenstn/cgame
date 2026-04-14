#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

enum Flags {
    IS_ACTIVE = 1<<0,
    FLAG_PLAYER_CONTROLLED = 1<<1,
    FLAG_CAN_MOVE = 1<<2,
    FLAG_PROJECTILE = 1<<3,
    FLAG_COLLIDES_WITH_WALL = 1<<4,

};


static size_t things_find_inactive(Thing* things) {
    for(int i = 1; i < MAX_THINGS; i++) {
        if (!flags_is_set(things[i].flags, IS_ACTIVE)) {
            return i;
        }
    }
    return 0;
}

static GameState *init(GameMemory* gameMemory) {

    GameState* state = (GameState*)gameMemory->permanent_storage;
    if (gameMemory->is_initialized) {
        return state;
    }

    printf("works? %s\n",gameMemory->platform_api.get_stuff());
    u8* arena_base = (u8*)gameMemory->permanent_storage + sizeof(GameState);

    arena_init(&state->permanent_arena, arena_base, gameMemory->permanent_storage_size - sizeof(GameState));

    state->things = arena_alloc(&state->permanent_arena, sizeof(Thing)*MAX_THINGS);
    for(int i = 0; i < MAX_THINGS; i++) {
        state->things[i].flags = 0;
        state->things[i].x = 0;
        state->things[i].y = 0;
        state->things[i].vx = 0;
        state->things[i].vy = 0;
        state->things[i].width = 0;
        state->things[i].height = 0;
    }
    for(int i = 1; i < 4; i++) {
        state->things[i].x = i*400;
        state->things[i].width = 20;
        state->things[i].height = 20;
        state->things[i].flags = IS_ACTIVE | FLAG_CAN_MOVE;
    }
    state->things[1].flags = FLAG_PLAYER_CONTROLLED | FLAG_CAN_MOVE | IS_ACTIVE;
    state->things[1].x = 400;
    state->things[1].y = 400;


    state->screenWidth = SCREEN_WIDTH;
    state->screenHeight = SCREEN_HEIGHT;
    state->levelWidth = 60;
    state->levelHeight = 30;
    state->tileSize = 100;
    state->level = arena_alloc(&state->permanent_arena, 60*30);

    state->keys_down = arena_alloc(&state->permanent_arena, _NUM_KEY_CODES);
    state->keys_down[KEY_UP] = SCANCODE_W;
    state->keys_down[KEY_LEFT] = SCANCODE_A;
    state->keys_down[KEY_DOWN] = SCANCODE_S;
    state->keys_down[KEY_RIGHT] = SCANCODE_D;
    state->keys_down[KEY_SHIFT] = SCANCODE_LSHIFT;

    state->image = gameMemory->platform_api.read_whole_file("test.bmp");
    state->image2 = gameMemory->platform_api.read_whole_file("test2.bmp");
    state->image3 = gameMemory->platform_api.read_whole_file("font.bmp");

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

static void drawPixel(GameState* state, int x, int y, uint32_t color) {
    state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
}

static void draw_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
        int x_end = _x+width;
        int y_end = _y+height;
        for(int x = _x; x< x_end;x++) {
            state->output_buffer[ARRAY_INDEX(x, _y, state->screenWidth)] = color;
        }
        for(int y = _y+1; y< y_end-1; y++) {
            state->output_buffer[ARRAY_INDEX(_x, y, state->screenWidth)] = color;
            state->output_buffer[ARRAY_INDEX(x_end, y, state->screenWidth)] = color;
        }
        for(int x = _x; x< x_end;x++) {
            state->output_buffer[ARRAY_INDEX(x, y_end, state->screenWidth)] = color;
        }
}

static void draw_partial_image(GameState* state, u8* image, int image_x, int image_y, int from_width, int from_height, int _x, int _y, int image_width, int image_height) {
        int x_end = image_x+from_width;
        int y_end = image_y+from_height;
        int out_y = y_end;
        for(int y = 8; y >0; y--) {
            int out_x = _x;
            if(y <0 || y >= state->screenHeight) {
                continue;
            }
            for(int x = 0; x < 10; x++) {
                if (x<0 || x>=state->screenWidth) {
                    continue;
                }
                int i = 138+ARRAY_INDEX(x, y, image_width);
                u8 b = image[i];
                u8 g = image[i+1];
                u8 r = image[i+2];
                u32 color = 0xff000000 | (b << 16) | (g << 8) | r;
                state->output_buffer[ARRAY_INDEX(700, 700, state->screenWidth)] = color;
                i+=3;
                out_x++;
            }
            out_y--;
        }
}

static void draw_image(GameState* state, u8* image, int _x, int _y, int image_width, int image_height) {
        int x_end = _x+image_width;
        int y_end = _y+image_height;
        int i = 138;
        for(int y = y_end; y >_y; y--) {
            if(y <0 || y >= state->screenHeight) {
                continue;
            }
            for(int x = _x; x < x_end; x++) {
                if (x<0 || x>=state->screenWidth) {
                    continue;
                }
                u8 b = image[i];
                u8 g = image[i+1];
                u8 r = image[i+2];
                u32 color = 0xff000000 | (b << 16) | (g << 8) | r;
                state->output_buffer[ARRAY_INDEX(x, y, state->screenWidth)] = color;
                i+=3;
            }
        }
}

static void fill_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
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

    /*
        for(int i = 0; i < 512; i++) {
            if (key_states[i]) {

            printf("Also: %d\n", (int)'a');
            printf("%d: %d\n", i, key_states[i]);
            }
        }
        */


        float speed = 5.0;

        MouseState* mouse = &state->mouse_state;

        state->viewportX = state->things[1].x+mouse->x*0.9-SCREEN_WIDTH;
        state->viewportY = state->things[1].y+mouse->y*0.9-SCREEN_HEIGHT;

        if (key_states[SCANCODE_LSHIFT]) {
            speed = 8;
        }

        if(mouse->left_button_click) {
            size_t index = things_find_inactive(state->things);
            Thing* bullet = &state->things[index];
            bullet->flags = IS_ACTIVE | FLAG_PROJECTILE | FLAG_CAN_MOVE;
            float angle = atan2(mouse->y - (-state->viewportY+state->things[1].y), mouse->x - (-state->viewportX+state->things[1].x));
            bullet->vx = state->things[1].vx + 5*cos(angle);
            bullet->vy = state->things[1].vy + 5*sin(angle);
            bullet->x = state->things[1].x;
            bullet->y = state->things[1].y;
            bullet->width=5;
            bullet->height=5;
            bullet->projectile_counter = 500;
        }
        // TODO Don't loop through all the things probably. Or just do the swapping thing for inactive
        for(int i = 0; i < MAX_THINGS; i++) {
            Thing* t = &state->things[i];
            if (!flags_is_set(t->flags, IS_ACTIVE)) {
                continue;
            }

            if(flags_is_set(t->flags, FLAG_PLAYER_CONTROLLED)) {
                    t->vx=0;
                    t->vy=0;
                if (key_states[SCANCODE_A]) {
                    t->vx=-speed;
                }
                if (key_states[SCANCODE_S]) {
                    t->vy=speed;
                }
                if (key_states[SCANCODE_D]) {
                    t->vx=speed;
                }
                if (key_states[SCANCODE_W]) {
                    t->vy=-speed;
                }
            }

            if (flags_is_set(t->flags, FLAG_CAN_MOVE)) {
                t->x += t->vx;
                t->y += t->vy;
            }

            if (flags_is_set(t->flags, FLAG_PROJECTILE)) {
                if (--t->projectile_counter <= 0) {
                    t->flags = flags_unset(t->flags, IS_ACTIVE);
                }
            }
        }

        state->things[2].y=200;
    //---------- Render 
    memset(state->output_buffer, 0, SCREEN_WIDTH*SCREEN_HEIGHT*4);

    int counter = 0;
    int start_x = clamp(ARRAY_INDEX(state->viewportX/100, 0, 60), 0, INT_MAX);
    int end_x = clamp(ARRAY_INDEX(state->viewportX/100+SCREEN_WIDTH/100, 0, 60)+1, 0, state->levelWidth);

    int start_y = ARRAY_INDEX(0, state->viewportY/6000, 60);
    int end_y = clamp(ARRAY_INDEX(0, state->viewportY/6000+SCREEN_HEIGHT/100, 60)+3, 0, state->levelHeight);
    //printf("pos: %d\n", start_y);
    //int start_y = clamp(state->viewportY, 0, INT_MAX);
    for(int y = start_y; y < end_y; y++) {
        for(int x = start_x; x < end_x; x++) {
            if(state->level[ARRAY_INDEX(x, y, 60)] == '1') {
                fill_rect(state, -state->viewportX+x*100, -state->viewportY+y*100, 100, 100, 0x33333333);
            } else if (state->level[ARRAY_INDEX(x, y, 60)] == '.') {
                fill_rect(state, -state->viewportX+x*100, -state->viewportY+y*100, 100, 100, 0x77777777);
            }
            counter++;
        }

    }
    //printf("Counter: %d\n", counter);
        draw_rect(state, state->mouse_state.x, state->mouse_state.y, 50, 50, 0xff0000ff);

    for(int i = 1; i < MAX_THINGS; i++) {
        Thing* t = &state->things[i];
        if (!flags_is_set(t->flags, IS_ACTIVE)) {
            continue;
        }
        uint32_t color = flags_is_set(t->flags, FLAG_PLAYER_CONTROLLED) ? 0xff00ff00 : 0x4f4f4f;
        if (flags_is_set(t->flags, FLAG_PROJECTILE)) {
            color = 0xffffffff;
        }
        fill_rect(state, -state->viewportX+t->x,-state->viewportY+t->y,t->width,t->height, color);
    }
    //printf("%f\n", (float)state->permanent_arena.used/(float)state->permanent_arena.size);
    draw_rect(state, 5, 5, 1000, 10, 0xffffffff);
    fill_rect(state, 6, 6, ((float)state->permanent_arena.used/(float)state->permanent_arena.size)*1000, 8, 0xafafafaf);

    u8* image = state->image;
    u8* image2 = state->image2;
    u8* image3 = state->image3;
    draw_image(state, image, 300, 300, 100, 100);
    draw_image(state, image2, 450, 300, 100, 100);
    draw_image(state, image3, 450, 500, 240, 8);
    draw_partial_image(state, image3,0,0,10,8, 600,600,240,8);
    return true;
}


static GameAPI api = {
    .init = init,
    .update_and_render = update_and_render
};

GameAPI* get_game_api() {
    return &api;
}


