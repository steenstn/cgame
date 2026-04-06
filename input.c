#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum Action {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    _NUM_ACTIONS,
} Action;

typedef struct KeyMapping {
    Action action;
    SDL_Scancode scancode;
} KeyMapping;

KeyMapping key_map[] = {
    {MOVE_LEFT, SDL_SCANCODE_A},
    {MOVE_RIGHT, SDL_SCANCODE_D},
    {MOVE_UP, SDL_SCANCODE_W},
    {MOVE_DOWN, SDL_SCANCODE_S},
};

bool keys_down[_NUM_ACTIONS]= {false};
