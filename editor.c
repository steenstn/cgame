#include "game.h"
#include <SDL2/SDL_keycode.h>
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
                    state->level.tiles[index] = '1';
                break;
                case TOOL_ERASE_WALL:
                    state->level.tiles[index] = '.';
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
