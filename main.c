#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
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
#include "engine.c"
#include "input.c"


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

/*
   void* lib = NULL; 
   while(true) {
       if(lib != NULL) {
           dlclose(lib);
       }
    lib = dlopen("./libname.so", RTLD_NOW | RTLD_GLOBAL);
    if(!lib) {
        printf("no: %s\n", dlerror());
        continue;
    } else {
    }


    char* (*yay_func)(void) = (char* (*)(void)) dlsym(lib, "yay");
    char* res = yay_func();
    printf("hells yeah: %s\n", res);
       
   }
   return 0;
   */
    printf("here\n");
    void* game_handle = NULL;
    game_handle = dlopen("./libgame.so", RTLD_NOW | RTLD_GLOBAL);
    if (!game_handle) {
        printf("Failed to load game.\n");
        exit(1);
    }
    GameAPI* (*get_api)() = dlsym(game_handle, "get_game_api");
    GameAPI* api = get_api();

    GameState* game_state = api->init();

    api->step(game_state);


    ImageSystem image_system = {.current_index = 1, .image_array = (Image[10]){}};

    srand(time(NULL));

    int screenWidth = 1600;
    int screenHeight = 1200;
    int32_t mouseX = 0;
    int32_t mouseY = 0;

    int viewportX = 0;
    int viewportY = 0;
    if (!wInit()) {
        return 1;
    }

    wWindow window = wCreateWindow(screenWidth, screenHeight);
    wRenderer renderer = wCreateRenderer(&window);

    wSetRenderDrawColor(&renderer, 0xff, 0xff, 0xff, 0xff);

    int level_width = 60;
    int level_height = 30;
    int tile_size = 100;
    char level[level_height*level_width];

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

    int player_image = loadImage(&renderer, &image_system, "cat.png", 100, 100);
    Thing things[2];
    for(int i = 0; i < 2; i++) {
        things[i] = (Thing){.x=520 + i*100, .y =120 + i*100, .image_index = player_image};
    }

    SDL_Event e;

    bool quit = false;


    const uint8_t* key_states = SDL_GetKeyboardState(NULL);


    float speed = 10;
int counter =0;
    while(quit == false) {


        if(counter++ %60 ==0){
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


        viewportX = mouseX+things[0].x-(float)screenWidth;
        viewportY = mouseY+things[0].y-(float)screenHeight;

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
            Thing* t = &things[i];
            t->oldx = t->x;
            t->oldy = t->y;

            if(keys_down[MOVE_UP]) {
                t->y -= speed;
            }
            if(keys_down[MOVE_DOWN]) {
                t->y += speed;
            }
            if(keys_down[MOVE_RIGHT]) {
                t->x += speed;
            }
            if(keys_down[MOVE_LEFT]) {
                t->x -= speed;
            }

            int arrayIndex= getArrayIndex(t->x, t->y, level_width, tile_size);
            if(level[arrayIndex] == '1') {
                t->x = t->oldx;
                t->y = t->oldy;
            }

            //wDrawImage(&renderer, things[i].image, -viewportX + things[i].x, -viewportY + things[i].y);
            wDrawImage(&renderer, &image_system.image_array[t->image_index], -viewportX + things[i].x, -viewportY + things[i].y);
        }
        wSetRenderDrawColor(&renderer, 100, 200, 200, 255);
        wDrawRect(&renderer, mouseX, mouseY, 10, 10);
        if(game_handle){
            api->step(game_state);
        }
        //printf("m: %d", game.game_state->mouseX);
        wRenderFrame(&renderer);
    }

    return 0;
}
