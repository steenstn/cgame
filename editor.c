#include "game.h"
#include <SDL2/SDL_keycode.h>

bool walls_contain(u8 value) {
    for(int i = 0; i < 16; i++) {
        if (walls[i]==value) {
            return true;
        }
    }
    return false;
}

static u8 set_tile_based_on_neighbours(GameState* state, int index) {
    u8 left_neighbour = state->level.tiles[index - 1];
    u8 right_neighbour = state->level.tiles[index + 1];
    u8 above_neighbour = state->level.tiles[index - state->level.level_width];
    u8 below_neighbour = state->level.tiles[index + state->level.level_width];
    int value = 0;
    if (walls_contain(left_neighbour)) value+=1;
    if (walls_contain(above_neighbour)) value+=2;
    if (walls_contain(right_neighbour)) value+=4;
    if (walls_contain(below_neighbour)) value+=8;
    switch(value) {
        case 0:
            state->level.tiles[index] = 'c';
        break;
        case 1:
            state->level.tiles[index] = 'f';
        break;
        case 2:
            state->level.tiles[index] = 'g';
        break;
        case 3:
            state->level.tiles[index] = '7';
        break;
        case 4:
            state->level.tiles[index] = 'd';
        break;
        case 5:
            state->level.tiles[index] = 'a';
        break;
        case 6:
            state->level.tiles[index] = '8';
        break;
        case 7:
            state->level.tiles[index] = '4';
        break;
        case 8:
            state->level.tiles[index] = 'e';
        break;
        case 9:
            state->level.tiles[index] = '6';
        break;
        case 10:
            state->level.tiles[index] = 'b';
        break;
        case 11:
            state->level.tiles[index] = '3';
        break;
        case 12:
            state->level.tiles[index] = '9';
        break;
        case 13:
            state->level.tiles[index] = '2';
        break;
        case 14:
            state->level.tiles[index] = '5';
        break;
        case 15:
            state->level.tiles[index] = '1';
        break;
    }
    return 0;
}

static void update_for_editor(GameState* state, const u8* key_states) {
        MouseState* mouse = &state->mouse_state;
            state->editor_state.mouse_position_level_index = ARRAY_INDEX((int)((state->viewportX+mouse->x)/state->level.tile_size), (int)((state->viewportY+mouse->y)/state->level.tile_size), state->level.level_width);
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
        if (state->keyboard_state.keys_hit[SCANCODE_1]) {
            state->editor_state.active_tool = state->editor_state.active_tool == TOOL_PLACE_WALL ? TOOL_ERASE_WALL : TOOL_PLACE_WALL;
        }
        if (state->keyboard_state.keys_hit[SCANCODE_2]) {
            state->editor_state.active_tool = TOOL_SELECT;
        }

        if (state->keyboard_state.keys_hit[SCANCODE_TAB] == 1) {
                bool res = state->platform_api.write_file("level.bin", state->level.tiles, state->level.level_width*state->level.level_height);
                if (!res) {
                    printf("Failed to save level\n");
                }
            state->mode = PLAY;
        }


        if(mouse->left_button_down) {
            int index = ARRAY_INDEX((int)((state->viewportX+mouse->x)/state->level.tile_size), (int)((state->viewportY+mouse->y)/state->level.tile_size), state->level.level_width);
            switch(state->editor_state.active_tool) {
                case TOOL_PLACE_WALL:
                    {
                        u8 left_neighbour = state->level.tiles[index - 1];
                        u8 right_neighbour = state->level.tiles[index + 1];
                        u8 above_neighbour = state->level.tiles[index - state->level.level_width];
                        u8 below_neighbour = state->level.tiles[index + state->level.level_width];

                        set_tile_based_on_neighbours(state, index);
                        if (left_neighbour != '.')set_tile_based_on_neighbours(state, index-1);
                        if (right_neighbour!= '.')set_tile_based_on_neighbours(state, index+1);
                        if (above_neighbour!= '.')set_tile_based_on_neighbours(state, index-state->level.level_width);
                        if (below_neighbour!= '.')set_tile_based_on_neighbours(state, index+state->level.level_width);
                    }
                break;
                case TOOL_ERASE_WALL:
                {

                    state->level.tiles[index] = '.';
                    u8 left_neighbour = state->level.tiles[index - 1];
                    u8 right_neighbour = state->level.tiles[index + 1];
                    u8 above_neighbour = state->level.tiles[index - state->level.level_width];
                    u8 below_neighbour = state->level.tiles[index + state->level.level_width];

                    if (left_neighbour != '.')set_tile_based_on_neighbours(state, index-1);
                    if (right_neighbour!= '.')set_tile_based_on_neighbours(state, index+1);
                    if (above_neighbour!= '.')set_tile_based_on_neighbours(state, index-state->level.level_width);
                    if (below_neighbour!= '.')set_tile_based_on_neighbours(state, index+state->level.level_width);
                }
                break;
                case TOOL_SELECT: {
                    int mx = state->viewportX+mouse->x;
                    int my = state->viewportY+mouse->y;
                    if (state->editor_state.thing_selected) {
                        state->things[state->editor_state.selected_thing_index].x = mx;
                        state->things[state->editor_state.selected_thing_index].y = my;
                    }
                    for(int i = 1; i < 3; i++) {
                        Thing* t = &state->things[i];
                        printf("Mouse %d %d\n", mx, my);
                        printf("thing[%d] %f %f\n", i, t->x, t->y);
                        if (aabb_collision(mx, my,1,1, t->x, t->y, t->width, t->height)) {
                            state->editor_state.thing_selected = true;
                            state->editor_state.selected_thing_index = i;
                        }
                    }
                break;
                }
            }
        } else {
            state->editor_state.thing_selected = false;
            state->editor_state.selected_thing_index = 0;
        }
}
