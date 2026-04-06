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

#include "sdl_wrapper.c"
#include "input.c"

typedef struct Thing {
    wRect rect;
    SDL_Texture* sdl_texture;
} Thing;


SDL_Rect mouse_rect= {.w=10, .h=10};

SDL_Rect viewport = {0, 0, 1600, 1200};

int main(void) {

    if (!wInit()) {
        return 1;
    }

    wWindow window = wCreateWindow(1600, 1200);
    wRenderer renderer = wCreateRenderer(&window);

    wSetRenderDrawColor(&renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_Surface* loadedSurface = IMG_Load("cat.png");


    int level_width = 30;
    int level_height = 30;
    int tile_size = 100;
    char level[level_height*level_width];

    for(int i = 0; i < level_width*level_height; i++) {
        level[i] = '.';
        if (i%level_width == 0 || i <= level_width || i > (level_width*level_height)-level_width || ((i+1)%(level_width))==0 || i%13==0) {
            level[i] = '1';
        }
    }

    Thing things[2];
    for(int i = 0; i < 2; i++) {
        things[i] = (Thing){{(SDL_Rect){120 + i*100, 120 + i*100, 100, 100}}, NULL};
        things[i].sdl_texture= SDL_CreateTextureFromSurface(renderer.renderer, loadedSurface);
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

        SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 0);

        SDL_RenderClear(renderer.renderer);

        SDL_Rect drawing_rect = {0,0,tile_size, tile_size};

        for(int i = 0; i < level_width*level_height;i++) {
            int drawing_x = (i%level_width)*tile_size;
            int drawing_y = (i/level_width)*tile_size;
            drawing_rect.x = drawing_x;
            drawing_rect.y = drawing_y;
            if(level[i] == '.') {
                wSetRenderDrawColor(&renderer, 100, 100, 100, 255);
            } else {
                wSetRenderDrawColor(&renderer, 20, 20, 10, 255);
            }
            wFillRect(&renderer, drawing_x, drawing_y, tile_size, tile_size);
            //SDL_RenderFillRect(renderer.renderer, &drawing_rect);

        }

        for(int i = 0; i<2;i++) {

            if(keys_down[MOVE_UP]) {
                things[i].rect.rect.y-=speed;
            }
            if(keys_down[MOVE_DOWN]) {
                things[i].rect.rect.y+=speed;
            }
            if(keys_down[MOVE_RIGHT]) {
                things[i].rect.rect.x+=speed;
            }
            if(keys_down[MOVE_LEFT]) {
                things[i].rect.rect.x-=speed;
            }
            wSetRenderDrawColor(&renderer, 100, 200, 200, 255);
            SDL_RenderDrawRect(renderer.renderer, &mouse_rect);
            SDL_RenderCopy(renderer.renderer,  things[i].sdl_texture, NULL, &things[i].rect.rect);
        }

        SDL_RenderPresent(renderer.renderer);
    }

    //SDL_DestroyWindow(window);
    //SDL_Quit();
    return 0;
}
