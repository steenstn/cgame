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
    float x;
    float y;
    Image* image;
} Thing;

typedef struct Vec2 {
    float x;
    float y;
} Vec2;


int main(void) {

    int32_t mouseX = 0;
    int32_t mouseY = 0;

    int viewportX = 0;
    int viewportY = 0;
    if (!wInit()) {
        return 1;
    }

    wWindow window = wCreateWindow(1600, 1200);
    wRenderer renderer = wCreateRenderer(&window);

    wSetRenderDrawColor(&renderer, 0xff, 0xff, 0xff, 0xff);


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
    Image image = loadImage(&renderer, "cat.png", 100, 100);
    for(int i = 0; i < 2; i++) {
        things[i] = (Thing){120 + i*100, 120 + i*100, &image};
    }

    SDL_Event e;

    bool quit = false;


    const uint8_t* key_states = SDL_GetKeyboardState(NULL);


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
                mouseX = e.motion.x;
                mouseY = e.motion.y;
            }
        }

        for(int i = 0; i < _NUM_ACTIONS; i++) {
            KeyMapping current_key_map = key_map[i];
            keys_down[current_key_map.action] = key_states[current_key_map.scancode];
        }

        wSetRenderDrawColor(&renderer, 0, 0, 0, 0);

        wRenderClear(&renderer);

        for(int i = 0; i < level_width*level_height;i++) {
            int drawing_x = (i%level_width)*tile_size;
            int drawing_y = (i/level_width)*tile_size;
            if(level[i] == '.') {
                wSetRenderDrawColor(&renderer, 100, 100, 100, 255);
            } else {
                wSetRenderDrawColor(&renderer, 20, 20, 10, 255);
            }
            wFillRect(&renderer, -viewportX+drawing_x, -viewportY+drawing_y, tile_size, tile_size);

        }

            if(keys_down[MOVE_UP]) {
                viewportY-=speed;
            }
            if(keys_down[MOVE_DOWN]) {
                viewportY+=speed;
            }
            if(keys_down[MOVE_RIGHT]) {
                viewportX+=speed;
            }
            if(keys_down[MOVE_LEFT]) {
                viewportX-=speed;
            }
        for(int i = 0; i<2;i++) {

            if(keys_down[MOVE_UP]) {
                things[i].y-=speed;
            }
            if(keys_down[MOVE_DOWN]) {
                things[i].y+=speed;
            }
            if(keys_down[MOVE_RIGHT]) {
                things[i].x+=speed;
            }
            if(keys_down[MOVE_LEFT]) {
                things[i].x-=speed;
            }
            wDrawImage(&renderer, things[i].image, -viewportX + things[i].x, -viewportY + things[i].y);
        }
        wSetRenderDrawColor(&renderer, 100, 200, 200, 255);
        wDrawRect(&renderer, mouseX, mouseY, 10, 10);

        wRenderFrame(&renderer);
    }

    return 0;
}
