//https://github.com/Pere001/2d-platformer-tutorial-2023
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

#include "game_engine.c"
#include "level.c"
#include "editor.c"
#define PI 3.1415926535

enum Flags {
    IS_ACTIVE = 1<<0,
    FLAG_PLAYER_CONTROLLED = 1<<1,
    FLAG_CAN_MOVE = 1<<2,
    FLAG_PROJECTILE = 1<<3,
    FLAG_COLLIDES_WITH_WALL = 1<<4,
    FLAG_AFFECTED_BY_GRAVITY = 1<<5,
    FLAG_AFFECTED_BY_FRICTION = 1<<6,
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
    arena_init(&state->frame_arena, gameMemory->transient_storage, gameMemory->transient_storage_size);

    state->things = arena_alloc(&state->permanent_arena, sizeof(Thing)*MAX_THINGS);
    for(int i = 0; i < MAX_THINGS; i++) {
        state->things[i].flags = 0;
        state->things[i].x = 0;
        state->things[i].y = 0;
        state->things[i].vx = 0;
        state->things[i].vy = 0;
        state->things[i].ax = 0;
        state->things[i].ay = 0;
        state->things[i].width = 0;
        state->things[i].height = 0;
    }
    for(int i = 1; i < 5; i++) {
        state->things[i].x = i*400;
        state->things[i].y = i*400;
        state->things[i].width = 20;
        state->things[i].height = 20;
        state->things[i].flags = IS_ACTIVE | FLAG_CAN_MOVE ;
    }

    // Init player
    state->things[1].flags = FLAG_PLAYER_CONTROLLED | FLAG_CAN_MOVE | IS_ACTIVE | FLAG_AFFECTED_BY_GRAVITY | FLAG_AFFECTED_BY_FRICTION;
    state->things[1].x = 400;
    state->things[1].y = 400;
    state->things[1].height = 40;


    state->platform_api = gameMemory->platform_api;

    state->screenWidth = SCREEN_WIDTH;
    state->screenHeight = SCREEN_HEIGHT;
    state->viewportX = 0;
    state->viewportY = 0;
    int level_width = 100;
    int level_height = 100;
    int tile_size = 32;
    state->level = (Level){.tiles = arena_alloc(&state->permanent_arena, level_width*level_height), NULL, .tile_size = tile_size, .level_width=level_width, .level_height = level_height};
    //state->level_visibility = arena_alloc(&state->permanent_arena, state->levelWidth*state->levelHeight);

    state->keyboard_state.keys_down = arena_alloc(&state->permanent_arena, _NUM_KEY_CODES);
    state->keyboard_state.keys_hit = arena_alloc(&state->permanent_arena, SDL_NUM_SCANCODES);
    state->keyboard_state.keys_down[KEY_UP] = SCANCODE_W;
    state->keyboard_state.keys_down[KEY_LEFT] = SCANCODE_A;
    state->keyboard_state.keys_down[KEY_DOWN] = SCANCODE_S;
    state->keyboard_state.keys_down[KEY_RIGHT] = SCANCODE_D;
    state->keyboard_state.keys_down[KEY_SHIFT] = SCANCODE_LSHIFT;

    state->mode = PLAY;

    state->editor_state = (EditorState) {.active_tool = TOOL_PLACE_WALL};

    state->mouse_state.left_button_down = 0;
    state->mouse_state.left_button_click = 0;


    state->image_list = arena_alloc(&state->permanent_arena, sizeof(Image) * 5);
    Image image = gameMemory->platform_api.load_image("tileset.bmp");
    state->image_list[0] = image;

    state->render_command_buffer.capacity = 1000;
    state->render_command_buffer.buffer = arena_alloc(&state->permanent_arena, sizeof(RenderCommand) * state->render_command_buffer.capacity);
    state->render_command_buffer.count = 0;

    u8* level = state->level.tiles;
    //u8* level_visibility = state->level_visibility;


    bool level_exists = gameMemory->platform_api.read_whole_file("level.bin", level, state->level.level_height*state->level.level_width);

    if (!level_exists) {
        printf("Creating default level\n");

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
    } else {
        printf("Loaded level\n");
    }

    return state;
}

//static void draw_image(GameState* state, u8* image, int _x, int _y, int image_width, int image_height) {
//}

static void update_for_game(GameState* state, const u8* key_states) {
        
        float speed = 0.2;
        float max_speed = 2;
        float max_y_speed = 3;

        MouseState* mouse = &state->mouse_state;

        state->viewportX = state->things[1].x-SCREEN_WIDTH*0.5;
        state->viewportY = state->things[1].y-SCREEN_HEIGHT*0.5;

        if (key_states[SCANCODE_LSHIFT]) {
            speed = 0.01;
        }
        if (state->keyboard_state.keys_hit[SCANCODE_TAB]) {
            state->mode = EDITOR;
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
            t->old_x = t->x;
            t->old_y = t->y;

            t->vx+= t->ax;
            t->vy+= t->ay;
            t->vx = clampf(t->vx, -max_speed, max_speed);
            //t->vy = clampf(t->vy, -max_speed, max_speed);
            t->x+= t->vx;
            t->y+= t->vy;

            if(flags_is_set(t->flags, FLAG_PLAYER_CONTROLLED)) {
                t->moving = false;
                    t->ax = 0;
                    t->ay = 0;
                if (key_states[SCANCODE_A]) {
                    t->moving = true;
                    t->ax=-speed;
                }
                if (key_states[SCANCODE_S]) {
                    t->moving = true;
                    t->ay=speed;
                }
                if (key_states[SCANCODE_D]) {
                    t->moving = true;
                    t->ax=speed;
                }
                if (key_states[SCANCODE_W]) {
                    if (!t->jumping) {
                        t->vy = -5;
                    }
                    t->jumping = true;
                }
            }

            if (flags_is_set(t->flags, FLAG_AFFECTED_BY_GRAVITY)) {
                t->vy+=0.1;
                if (t->vy > max_y_speed) {
                    t->vy = max_y_speed;
                }
                
            }

            if (flags_is_set(t->flags, FLAG_CAN_MOVE)) {
                t->x += t->vx;
                t->y += t->vy;

                //TODO Funkar inte alltid, ibland fastnar man
                if (t->vx >0) {
                    if (level_get_tile(&state->level, t->x + t->width, t->y+(int)(t->height/2)) == '1') {
                        t->x = t->old_x;
                    }
                } else if (t->vx < 0) {
                    if (level_get_tile(&state->level, t->x - 1, t->y+(int)(t->height/2)) == '1') {
                        t->x = t->old_x;
                    }
                }
                if (t->vy >0) {
                    if (level_get_tile(&state->level, t->x+(int)(t->width/2), t->y+t->height)== '1') {
                        t->y = t->old_y;
                        t->jumping = false;
                    }
                } else if (t->vy < 0) {
                    if (level_get_tile(&state->level, t->x+(int)(t->width/2), t->y)== '1') {
                        t->y = t->old_y;
                        t->vy = 0;
                        t->ay = 0;
                    }
                }
            }
            if (flags_is_set(t->flags, FLAG_AFFECTED_BY_FRICTION)) {
                if (!t->moving && level_get_tile(&state->level, t->x, t->y+t->height) == '1') {
                    t->vx/=1.2;
                }
                if (!t->moving && level_get_tile(&state->level, t->x, t->y+t->height == '.')) {
                    t->vx/=1.1;
                }
            }


            if (flags_is_set(t->flags, FLAG_PROJECTILE)) {
                if (--t->projectile_counter <= 0) {
                    t->flags = flags_unset(t->flags, IS_ACTIVE);
                }
            }
        }

}


static void print_scancodes(const u8* key_states) {
    for(int i = 0; i < 512; i++) {
        if (key_states[i]) {

        printf("Also: %d\n", (int)'a');
        printf("%d: %d\n", i, key_states[i]);
        }
    }
}

static bool update_and_render(GameState* state, const u8* key_states) {
    arena_clear(&state->frame_arena);
    switch (state->mode) {
        case PLAY:
            update_for_game(state, key_states);
        break;
        case EDITOR:
            update_for_editor(state, key_states);
        break;
    }
    state->render_command_buffer.count = 0;
        /*for(int i = 0; i < _NUM_KEY_CODES; i++) {
            printf("lol: %d", key_states[i]);
            state->keys_down[i] = key_states[state->keys_down[i]];
            printf("state->keys_down[%d]: %d\n",i, state->keys_down[i]);
        }
        */

    
        //print_scancodes(key_states);

    //---------- Render 
    render_command_push_clear(&state->render_command_buffer);
    //memset(state->level_visibility, 0, state->levelWidth*state->levelHeight);



    int tile_size = state->level.tile_size;
    int level_width = state->level.level_width;
    int level_height = state->level.level_height;
    int start_x = clamp(ARRAY_INDEX(state->viewportX/tile_size, 0, level_width), 0, INT_MAX);
    int end_x = clamp(ARRAY_INDEX(state->viewportX/tile_size+SCREEN_WIDTH/tile_size, 0, level_width)+2, 0, level_width);

    int start_y = ARRAY_INDEX(0, state->viewportY/(level_width*tile_size), level_width);
    int end_y = clamp((state->viewportY+SCREEN_HEIGHT) / tile_size+1, 0, level_height);


    for(int y = start_y; y < end_y; y++) {
        for(int x = start_x; x < end_x; x++) {
            int drawing_x = -state->viewportX+x*tile_size;
            int drawing_y = -state->viewportY+y*tile_size;
            int index = ARRAY_INDEX(x, y, level_width);

            if(state->level.tiles[index] == '1') {
                draw_cropped_image(state, 0, (x%3)*32, 0, 32, 32, drawing_x, drawing_y, 32, 32);
                //fill_rect(state, -state->viewportX+x*state->tileSize, -state->viewportY+y*tile_size, tile_size, tile_size, 0x33333333);
            } else if (state->level.tiles[ARRAY_INDEX(x, y, level_width)] == '.') {
                draw_cropped_image(state, 0, 8*32, 7*32, 32, 32, drawing_x, drawing_y, 32, 32);
                //fill_rect(state, -state->viewportX+x*state->tileSize, -state->viewportY+y*tile_size, tile_size, tile_size, 0x77777777);
            }
        }
    
}

    //printf("Counter: %d\n", counter);

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
    draw_rect(state, 100, 5, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 6, ((float)state->permanent_arena.used/(float)state->permanent_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 20, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 21, ((float)state->frame_arena.used/(float)state->frame_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 30, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 31, ((float)state->render_command_buffer.count/(float)state->render_command_buffer.capacity)*600, 8, 0xafafafaf);

    if (state->mode == EDITOR) {
        fill_rect(state, 20, 20, 20, 20, 0xff73af13);
    }


    //render_command_push_draw_image(&state->render_command_buffer, state->image_list[0], 450, 400);
    return true;
}


static GameAPI api = {
    .init = init,
    .update_and_render = update_and_render
};

GameAPI* get_game_api() {
    return &api;
}


