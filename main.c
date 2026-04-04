#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>

#include "input.c"

typedef struct Thing {
    SDL_Rect sdl_rect;
    SDL_Texture* sdl_texture;
} Thing;

SDL_Rect mouse_rect= {.w=10, .h=10};


int main(void) {

    Thing things[2];
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 1200, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        return 1;
    }


    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image fail");
        exit(1);
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_Surface* loadedSurface = IMG_Load("cat.png");
    for(int i = 0; i < 2; i++) {
        things[i] = (Thing){(SDL_Rect){20 + i*100, 40 + i*100, 100, 100}, NULL};
        things[i].sdl_texture= SDL_CreateTextureFromSurface(renderer, loadedSurface);
    }

    SDL_FreeSurface(loadedSurface);
    
    SDL_Event e;

    bool quit = false;

    const Uint8* key_states = SDL_GetKeyboardState(NULL);

    float speed = 10;
    while(quit == false) {

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
                mouse_rect.x = e.motion.x;
                mouse_rect.y = e.motion.y;
            }
        }

        for(int i = 0; i < _NUM_ACTIONS; i++) {
            KeyMapping current_key_map = key_map[i];
            keys_down[current_key_map.action] = key_states[current_key_map.scancode];
        }

        SDL_SetRenderDrawColor(renderer,
                       0,
                       0,
                       0,
                       0);
        SDL_RenderClear(renderer);
        for(int i = 0; i<2;i++) {

            if(keys_down[MOVE_UP]) {
                things[i].sdl_rect.y-=speed;
            }
            if(keys_down[MOVE_DOWN]) {
                things[i].sdl_rect.y+=speed;
            }
            if(keys_down[MOVE_RIGHT]) {
                things[i].sdl_rect.x+=speed;
            }
            if(keys_down[MOVE_LEFT]) {
                things[i].sdl_rect.x-=speed;
            }
        SDL_SetRenderDrawColor(renderer,
                       100,
                       200,
                       200,
                       255);
            SDL_RenderDrawRect(renderer, &mouse_rect);
            SDL_RenderCopy(renderer,  things[i].sdl_texture, NULL, &things[i].sdl_rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
