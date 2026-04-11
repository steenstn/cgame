// https://yakvi.github.io/handmade-hero-notes/html/day11.html
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dlfcn.h>

#include "game.h"


typedef struct Vec2 {
    float x;
    float y;
} Vec2;


int getArrayIndex(int x, int y, int levelWidth, int tileWidth) {
    int xPos = floorf((float)x/tileWidth);
    int yPos = levelWidth * floorf((float)y/tileWidth);
    return xPos + yPos;
}

int main(void) {

    srand(time(NULL));
    void* game_handle = NULL;
    game_handle = dlopen("./libgame.so", RTLD_NOW | RTLD_GLOBAL);
    if (!game_handle) {
        printf("Failed to load game.\n");
        exit(1);
    }
    GameAPI* (*get_api)() = dlsym(game_handle, "get_game_api");
    GameAPI* api = get_api();

    GameMemory game_memory = {0};
    game_memory.permanent_storage_size = 1024 * 1024 * 200;
    game_memory.permanent_storage = malloc(game_memory.permanent_storage_size);

    GameState* game_state = api->init(&game_memory);

    int screenWidth = SCREEN_WIDTH;
    int screenHeight = SCREEN_HEIGHT;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize: %s\n", SDL_GetError());
        exit(1);
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image fail");
        exit(1);
    }

    //SDL_ShowCursor(SDL_DISABLE);

    SDL_Window* window = SDL_CreateWindow("SDL tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);

    SDL_Event e;

    bool quit = false;
    int counter =0;
    while(quit == false) {

        if(counter++ %30 == 0){
            if(game_handle) {
                dlclose(game_handle);
            }
            game_handle = dlopen("./libgame.so", RTLD_NOW | RTLD_GLOBAL);
            if(!game_handle) {
                printf("Failed to hot reload\n");
            } else {
                GameAPI* (*get_api)() = dlsym(game_handle, "get_game_api");
                GameAPI* new_api = get_api();
                api = new_api;
            }
        }

        game_state->mouse_state.left_button_click = false;
        game_state->mouse_state.left_button_click = false;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                    break;
                }
            }
            if (e.type == SDL_MOUSEMOTION) {
                game_state->mouse_state.x = e.motion.x;
                game_state->mouse_state.y = e.motion.y;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    game_state->mouse_state.left_button_down = true;
                    game_state->mouse_state.left_button_click = true;

                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    game_state->mouse_state.right_button_down = true;
                    game_state->mouse_state.right_button_click = true;
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    game_state->mouse_state.left_button_down = false;
                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    game_state->mouse_state.right_button_down = false;
                }
            }
        }
    
        int size = 0;
        const uint8_t* key_states = SDL_GetKeyboardState(&size);
        //printf("size %d\n", size);

        if(game_handle){
            api->update_and_render(game_state, key_states);
        }

        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);

        memcpy(pixels, game_state->output_buffer, screenWidth*screenHeight*4);
        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

    }

    free(game_memory.permanent_storage);

    return 0;
}
