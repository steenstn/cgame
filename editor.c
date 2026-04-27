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
        // TODO add flip
        if (state->keyboard_state.keys_hit[SCANCODE_1]) {
            printf("aa\n");
            state->editor_state.active_tool = state->editor_state.active_tool == TOOL_PLACE_WALL ? TOOL_ERASE_WALL : TOOL_PLACE_WALL;
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
            }
        }
}
