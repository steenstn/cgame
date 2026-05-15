//https://github.com/Pere001/2d-platformer-tutorial-2023
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    FLAG_AGGRESSIVE = 1<<7
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
    arena_init(&state->frame_arena, gameMemory->transient_storage, gameMemory->transient_storage_size/2);
    arena_init(&state->scratch_arena, gameMemory->transient_storage, gameMemory->transient_storage_size/2);

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

    // Init player
    state->things[1].flags = FLAG_PLAYER_CONTROLLED | FLAG_CAN_MOVE | IS_ACTIVE | FLAG_AFFECTED_BY_GRAVITY | FLAG_AFFECTED_BY_FRICTION;
    state->things[1].x = 400;
    state->things[1].y = 400;
    state->things[1].width = 20;
    state->things[1].height = 30;

    for(int i = 2; i < 3; i++) {
        state->things[i].x = 600;
        state->things[i].y = 500;
        state->things[i].width = 20;
        state->things[i].height = 20;
        state->things[i].flags = IS_ACTIVE | FLAG_CAN_MOVE | FLAG_AGGRESSIVE;
    }

    state->platform_api = gameMemory->platform_api;
    state->ms_elapsed = 0;

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

    __builtin_dump_struct(&state->editor_state, printf);
    state->mouse_state.left_button_down = 0;
    state->mouse_state.left_button_click = 0;


    state->image_list = arena_alloc(&state->permanent_arena, sizeof(Image) * 5);
    Image image = gameMemory->platform_api.load_image("tileset.png");
    Image player_image = gameMemory->platform_api.load_image("player.png");
    state->image_list[0] = image;
    state->image_list[1] = player_image;

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
//


vec2 index_to_vec2(int index, int width) {
    return ((vec2){index % width, (float)index / (float)width});
}

static void handle_neighbour(Dict* came_from, Queue* frontier,  vec2 current, int neighbour, int level_width, int tile_size) {

        if (!dict_has_key(came_from, neighbour)) {
            queue_push(frontier, neighbour);
            dict_set_value(came_from, neighbour, ARRAY_INDEX((float)(current.x),(float) (current.y), level_width));
        }
}

typedef struct IntList {
    int size;
    int* entries;
} IntList;

static IntList get_path(GameState* state, vec2 start_position, vec2 goal) {
    Queue frontier = queue_init(&state->scratch_arena, 1000);
    int level_width = state->level.level_width;
    int tile_size = state->level.tile_size;
    int start_index = ARRAY_INDEX((float)(start_position.x/tile_size), (float)(start_position.y/tile_size), level_width);
    queue_push(&frontier, start_index);

    Dict came_from = dict_init(&state->scratch_arena, 1000);
    dict_set_value(&came_from, start_index, -1);

    int lol = 0;
    while(frontier.size > 0) {
        if(lol++>10) break;
        //printf("frontier size before pop: %d\n", frontier.size);
        int current = queue_pop(&frontier);

        //printf("frontier size aftre pop: %d\n", frontier.size);
        /*
        if (ARRAY_INDEX(goal.x/tile_size, goal.y/tile_size, level_width) == current) {
            printf("end\n");
            break;
        }*/
        //__builtin_dump_struct(&current, printf);

        vec2 current_vec = index_to_vec2(current, level_width);
        int above = ARRAY_INDEX((float)(current_vec.x), (float)((current_vec.y-1)), level_width);
        //int below = ARRAY_INDEX(current_vec.x, current_vec.y+tile_size, level_width);
        //int left = ARRAY_INDEX(current_vec.x-tile_size, current_vec.y, level_width );
        //int right = ARRAY_INDEX(current_vec.x+tile_size, current_vec.y, level_width );
        handle_neighbour(&came_from, &frontier, current_vec, above, level_width, tile_size);
        //handle_neighbour(&came_from, &frontier, current_vec, below, level_width, tile_size);
        //handle_neighbour(&came_from, &frontier, current_vec, left, level_width, tile_size);
        //handle_neighbour(&came_from, &frontier, current_vec, right, level_width, tile_size);
        //printf("frontier now: %d\n", frontier.size);
    }

    for(int i = 0; i < came_from.size; i++) {
        //printf("lol\n");
        state->level.tiles[came_from.entries[i].key] = '2';
    }

    
    arena_clear(&state->scratch_arena);

return (IntList){};

}

static void update_for_game(GameState* state, const u8* key_states, float delta_time) {
        
        float speed = 0.1;
        float max_speed = 2;
        float max_y_speed = 3;

        MouseState* mouse = &state->mouse_state;

        state->viewportX = state->things[1].x-SCREEN_WIDTH*0.5;
        state->viewportY = state->things[1].y-SCREEN_HEIGHT*0.5;
        state->viewportX = clamp(state->viewportX, 0, state->level.level_width*state->level.tile_size-SCREEN_WIDTH);
        state->viewportY = clamp(state->viewportY, 0, state->level.level_height*state->level.tile_size-SCREEN_HEIGHT);

        if (key_states[SCANCODE_LSHIFT]) {
            speed = 1;
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
            if (t->vx > 0 ) {
                t->direction = RIGHT;
            } else if (t->vx < 0) {
                t->direction = LEFT;
            }
            //t->vy = clampf(t->vy, -max_speed, max_speed);
            t->x+= t->vx*delta_time;
            t->y+= t->vy*delta_time;

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
                        t->vy = -4;
                    }
                    t->jumping = true;
                }
            }

            if (flags_is_set(t->flags, FLAG_AGGRESSIVE)) {
                Thing* player = &state->things[1];
                get_path(state, (vec2){t->x, t->y},(vec2){player->x, player->y});
                if (player->x < t->x) {
                    //t->vx = -1;
                } else if(player->x > t->x) {
                    //t->vx = 1;
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

static bool update_and_render(GameState* state, const u8* key_states, u64 ms_elapsed) {
    float delta_time = (ms_elapsed - state->ms_elapsed) / 1000.0;
    state->ms_elapsed = ms_elapsed;
    printf("dt: %f\n", delta_time);
    arena_clear(&state->frame_arena);
    switch (state->mode) {
        case PLAY:
            update_for_game(state, key_states, delta_time);
        break;
        case EDITOR:
            update_for_editor(state, key_states);
        break;
    }
        /*for(int i = 0; i < _NUM_KEY_CODES; i++) {
            printf("lol: %d", key_states[i]);
            state->keys_down[i] = key_states[state->keys_down[i]];
            printf("state->keys_down[%d]: %d\n",i, state->keys_down[i]);
        }
        */

    
        //print_scancodes(key_states);

    Queue queue = {};
    queue.entries = arena_alloc(&state->frame_arena, 20);

    //---------- Render 
    state->render_command_buffer.count = 0;
    render_command_push_clear(&state->render_command_buffer);
    fill_rect(state, 0,0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00ffbf20);
    //memset(state->level_visibility, 0, state->levelWidth*state->levelHeight);



    int tile_size = state->level.tile_size;
    int level_width = state->level.level_width;
    int level_height = state->level.level_height;
    int start_x = clamp(ARRAY_INDEX(state->viewportX/tile_size, 0, level_width), 0, INT_MAX);
    int end_x = clamp(ARRAY_INDEX(state->viewportX/tile_size+SCREEN_WIDTH/tile_size, 0, level_width)+2, 0, level_width);

    int start_y = ARRAY_INDEX(0, state->viewportY/(level_width*tile_size), level_width);
    int end_y = clamp((state->viewportY+SCREEN_HEIGHT) / tile_size+1, 0, level_height);


    int level_image_index = 0;
    for(int y = start_y; y < end_y; y++) {
        for(int x = start_x; x < end_x; x++) {
            int drawing_x = -state->viewportX+x*tile_size;
            int drawing_y = -state->viewportY+y*tile_size;
            int index = ARRAY_INDEX(x, y, level_width);

            int tile_offset_x = -1;
            int tile_offset_y = -1;
            switch(state->level.tiles[index]) {
                case '1':
                    tile_offset_x = 3*tile_size;
                    tile_offset_y = 0;
                break;
                case '2':
                    tile_offset_x = 9*tile_size;
                    tile_offset_y = 7*tile_size;
                break;
            }

            if (tile_offset_x!= -1 && tile_offset_y != -1) {
                draw_cropped_image(state, level_image_index, tile_offset_x, tile_offset_y, tile_size, tile_size, drawing_x, drawing_y, tile_size, tile_size);
            }

        }
    
}

    //printf("Counter: %d\n", counter);

    for(int i = 1; i < MAX_THINGS; i++) {
        Thing* t = &state->things[i];
        if (!flags_is_set(t->flags, IS_ACTIVE)) {
            continue;
        }
        uint32_t color = 0x4f4fff;
        if (flags_is_set(t->flags, FLAG_PROJECTILE)) {
            color = 0xffffffff;
        }

        
        if(flags_is_set(t->flags, FLAG_PLAYER_CONTROLLED)) {
            draw_cropped_image(state, 1, 0, 30-30*t->direction, t->width, t->height, -state->viewportX+t->x, -state->viewportY+t->y, state->image_list[1].width, state->image_list[1].height);
        } else {
            fill_rect(state, -state->viewportX+t->x,-state->viewportY+t->y,t->width,t->height, color);
        }

    }
    //printf("%f\n", (float)state->permanent_arena.used/(float)state->permanent_arena.size);
    draw_rect(state, 100, 5, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 6, ((float)state->permanent_arena.used/(float)state->permanent_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 20, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 21, ((float)state->frame_arena.used/(float)state->frame_arena.size)*600, 8, 0xafffafaf);
    draw_rect(state, 100, 30, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 31, ((float)state->scratch_arena.used/(float)state->scratch_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 40, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 51, ((float)state->render_command_buffer.count/(float)state->render_command_buffer.capacity)*600, 8, 0xafafafff);

    //__builtin_dump_struct(&state->things[2], printf);
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


