#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

#include "game_engine.h"

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
    arena_init(&state->frame_arena, gameMemory->transient_storage, gameMemory->transient_storage_size);

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
    for(int i = 1; i < 3; i++) {
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
    state->level = arena_alloc(&state->permanent_arena, state->levelWidth*state->levelHeight);

    state->keyboard_state.keys_down = arena_alloc(&state->permanent_arena, _NUM_KEY_CODES);
    state->keyboard_state.keys_hit = arena_alloc(&state->permanent_arena, SDL_NUM_SCANCODES);
    state->keyboard_state.keys_down[KEY_UP] = SCANCODE_W;
    state->keyboard_state.keys_down[KEY_LEFT] = SCANCODE_A;
    state->keyboard_state.keys_down[KEY_DOWN] = SCANCODE_S;
    state->keyboard_state.keys_down[KEY_RIGHT] = SCANCODE_D;
    state->keyboard_state.keys_down[KEY_SHIFT] = SCANCODE_LSHIFT;

    state->mode = PLAY;


    state->image_list = arena_alloc(&state->permanent_arena, sizeof(Image) * 5);
    Image image = gameMemory->platform_api.load_image("test2.bmp");
    state->image_list[0] = image;

    state->render_command_buffer.capacity = 1000;
    state->render_command_buffer.buffer = arena_alloc(&state->frame_arena, sizeof(RenderCommand) * state->render_command_buffer.capacity);
    state->render_command_buffer.count = 0;

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

static void render_command_push_draw_rect(RenderCommands* buffer, int x, int y, int w, int h, u32 color) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_DRAW_RECT;
        cmd->data.fill_rect.x = x;
        cmd->data.fill_rect.y = y;
        cmd->data.fill_rect.w = w;
        cmd->data.fill_rect.h = h;
        cmd->data.fill_rect.color = color;
}

static void render_command_push_draw_image(RenderCommands* buffer, Image image, int x, int y) {
    RenderCommand* cmd = &buffer->buffer[buffer->count++];
    cmd->type = RC_DRAW_IMAGE;
    cmd->data.draw_image.x = x;
    cmd->data.draw_image.y = y;
    cmd->data.draw_image.width = image.width;
    cmd->data.draw_image.height = image.height;
    cmd->data.draw_image.image = image.image;
}

static void render_command_push_draw_partial_image(RenderCommands* buffer, Image image, int image_x, int image_y, int partial_width, int partial_height, int x, int y) {
    RenderCommand* cmd = &buffer->buffer[buffer->count++];
    cmd->type = RC_DRAW_CROPPED_IMAGE;
    cmd->data.draw_image.x = x;
    cmd->data.draw_image.y = y;
    cmd->data.draw_image.image_x = image_x;
    cmd->data.draw_image.image_y = image_y;
    cmd->data.draw_image.crop_width = partial_width;
    cmd->data.draw_image.crop_height = partial_height;
    cmd->data.draw_image.width = image.width;
    cmd->data.draw_image.height = image.height;
    cmd->data.draw_image.image = image.image;
}

static void draw_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
        render_command_push_draw_rect(&state->render_command_buffer, _x, _y, width, height, color);
}

static void draw_cropped_image(GameState* state, u8* image, int image_x, int image_y, int from_width, int from_height, int _x, int _y, int image_width, int image_height) {
        render_command_push_draw_partial_image(&state->render_command_buffer, state->image_list[0], image_x, image_y, from_width, from_height, _x, _y);
}

static void draw_image(GameState* state, u8* image, int _x, int _y, int image_width, int image_height) {
}


static void render_command_push_fill_rect(RenderCommands* buffer, int x, int y, int w, int h, u32 color) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_FILL_RECT;
        cmd->data.fill_rect.x = x;
        cmd->data.fill_rect.y = y;
        cmd->data.fill_rect.w = w;
        cmd->data.fill_rect.h = h;
        cmd->data.fill_rect.color = color;
}

static void render_command_push_clear(RenderCommands* buffer) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_CLEAR;
}


static void fill_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
        //state->render_command_buffer.buffer[state->render_command_buffer.length++] = *(RenderCommand*)arena_alloc(&state->frame_arena, sizeof(RenderCommand));
        render_command_push_fill_rect(&state->render_command_buffer, _x, _y, width, height, color);
}

static void update_for_game(GameState* state, const u8* key_states) {

        float speed = 5.0;

        MouseState* mouse = &state->mouse_state;

        state->viewportX = state->things[1].x+mouse->x*0.9-SCREEN_WIDTH;
        state->viewportY = state->things[1].y+mouse->y*0.9-SCREEN_HEIGHT;

        if (key_states[SCANCODE_LSHIFT]) {
            speed = 8;
        }
        if (state->keyboard_state.keys_hit[SDLK_TAB]) {
            if(state->mode == PLAY) {
                state->mode = EDITOR;
            } else if(state->mode == EDITOR) {
                state->mode = PLAY;
            }
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

                //TODO Funkar inte alltid, ibland fastnar man
                if (t->vx >0) {
                    int index = ARRAY_INDEX((int)((t->x+1)/100), (int)(t->y/100), state->levelWidth);
                    if (state->level[index] == '1') {
                        t->x = t->old_x;
                    }
                } else if (t->vx < 0) {
                    int index = ARRAY_INDEX((int)((t->x-1)/100), (int)(t->y/100), state->levelWidth);
                    if (state->level[index] == '1') {
                        t->x = t->old_x;
                    }
                }
                if (t->vy >0) {
                    int index = ARRAY_INDEX((int)((t->x)/100), (int)((t->y+1)/100), state->levelWidth);
                    if (state->level[index] == '1') {
                        t->y = t->old_y;
                    }
                } else if (t->vy < 0) {
                    int index = ARRAY_INDEX((int)((t->x)/100), (int)((t->y-1)/100), state->levelWidth);
                    if (state->level[index] == '1') {
                        t->y = t->old_y;
                    }
                }
            }


            if (flags_is_set(t->flags, FLAG_PROJECTILE)) {
                if (--t->projectile_counter <= 0) {
                    t->flags = flags_unset(t->flags, IS_ACTIVE);
                }
            }
        }

        state->things[2].y=200;
}


static void update_for_editor(GameState* state, const u8* key_states) {
        MouseState* mouse = &state->mouse_state;
        int speed = 9;
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

        if (state->keyboard_state.keys_hit[SDLK_TAB]) {
            state->mode = PLAY;
        }

        if(mouse->left_button_click) {
            int index = ARRAY_INDEX((int)((state->viewportX+mouse->x)/state->tileSize), (int)((state->viewportY+mouse->y)/state->tileSize), state->levelWidth);
            state->level[index] = '2';
        }
}

static bool update_and_render(GameState* state, const u8* key_states) {
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

    
        /*for(int i = 0; i < 512; i++) {
            if (key_states[i]) {

            printf("Also: %d\n", (int)'a');
            printf("%d: %d\n", i, key_states[i]);
            }
        }*/


    //---------- Render 
    render_command_push_clear(&state->render_command_buffer);

    int counter = 0;

    int start_x = clamp(ARRAY_INDEX(state->viewportX/state->tileSize, 0, state->levelWidth), 0, INT_MAX);
    int end_x = clamp(ARRAY_INDEX(state->viewportX/state->tileSize+SCREEN_WIDTH/state->tileSize, 0, state->levelWidth)+2, 0, state->levelWidth);

    int start_y = ARRAY_INDEX(0, state->viewportY/(state->levelWidth*state->tileSize), state->levelWidth);
    int end_y = clamp(ARRAY_INDEX(0, state->viewportY/state->tileSize+SCREEN_HEIGHT, state->levelWidth),0, INT_MAX );
    //printf("pos: %d\n", start_y);
    //int start_y = clamp(state->viewportY, 0, INT_MAX);
    for(int y = start_y; y < end_y; y++) {
        for(int x = start_x; x < end_x; x++) {
            if(state->level[ARRAY_INDEX(x, y, state->levelWidth)] == '1') {
                fill_rect(state, -state->viewportX+x*state->tileSize, -state->viewportY+y*100, 100, 100, 0x33333333);
            } else if (state->level[ARRAY_INDEX(x, y, state->levelWidth)] == '.') {
                fill_rect(state, -state->viewportX+x*state->tileSize, -state->viewportY+y*100, 100, 100, 0x77777777);
            } else if (state->level[ARRAY_INDEX(x, y, state->levelWidth)] == '2') {
                fill_rect(state, -state->viewportX+x*state->tileSize, -state->viewportY+y*100, 100, 100, 0xffaa3322);
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
    draw_rect(state, 100, 5, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 6, ((float)state->permanent_arena.used/(float)state->permanent_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 20, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 21, ((float)state->frame_arena.used/(float)state->frame_arena.size)*600, 8, 0xafafafaf);
    draw_rect(state, 100, 30, 1000, 10, 0xffffffff);
    fill_rect(state, 101, 31, ((float)state->render_command_buffer.count/(float)state->render_command_buffer.capacity)*600, 8, 0xafafafaf);

    if (state->mode == EDITOR) {
        fill_rect(state, 20, 20, 20, 20, 0xff73af13);
    }

    render_command_push_draw_image(&state->render_command_buffer, state->image_list[0], 450, 400);
    return true;
}


static GameAPI api = {
    .init = init,
    .update_and_render = update_and_render
};

GameAPI* get_game_api() {
    return &api;
}


