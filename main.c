// https://yakvi.github.io/handmade-hero-notes/html/day11.html
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dlfcn.h>

#include "game.h"

typedef struct Textures {
    int count;
    SDL_Texture** textures;
} Textures;

int getArrayIndex(int x, int y, int levelWidth, int tileWidth) {
    int xPos = floorf((float)x/tileWidth);
    int yPos = levelWidth * floorf((float)y/tileWidth);
    return xPos + yPos;
}

void* platform_read_whole_file(char* path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("Failed to load %s\n", path);
        exit(1);
    }
    void* file = malloc(1024*1024);
    fread(file, 1024*1024, 1, fp);
    fclose(fp);
    return file;
}

SDL_Renderer* renderer;

Image platform_load_image(char* path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    Image image = {texture, .width =w, .height = h};
    return image;
}

char* woop() {
    return "yay";
}


int main(void) {

    srand(time(NULL));

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
    
    float scale = 1;
    SDL_Window* window = SDL_CreateWindow("SDL tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth*scale, screenHeight*scale, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, screenWidth, screenHeight);


    SDL_Event e;
    void* game_handle = NULL;
    game_handle = dlopen("./libgame.so", RTLD_NOW | RTLD_GLOBAL);
    if (!game_handle) {
        printf("Failed to load game.\n");
        printf("%s\n", SDL_GetError());
        exit(1);
    }
    GameAPI* (*get_api)() = dlsym(game_handle, "get_game_api");
    GameAPI* api = get_api();


    GameMemory game_memory = {0};
    game_memory.permanent_storage_size = 1024 * 1024 * 10;
    game_memory.permanent_storage = malloc(game_memory.permanent_storage_size);
    game_memory.transient_storage_size = 1024 * 1024 * 2;
    game_memory.transient_storage = malloc(game_memory.transient_storage_size);
    game_memory.platform_api.get_stuff = woop;
    game_memory.platform_api.read_whole_file = platform_read_whole_file;
    game_memory.platform_api.load_image = platform_load_image;

    GameState* game_state = api->init(&game_memory);

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

        for(int i = 0; i < SDL_NUM_SCANCODES;i++){
            game_state->keyboard_state.keys_hit[i] = 0;
        }
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                    break;
                    case SDLK_TAB:
                        game_state->keyboard_state.keys_hit[SDLK_TAB]++;
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

        int num_commands = game_state->render_command_buffer.count;
        for(int i = 0; i < num_commands; i++) {
            RenderCommand command = game_state->render_command_buffer.buffer[i];
            switch(command.type) {
                case RC_CLEAR: 
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  
                    SDL_RenderClear(renderer);
                break;
                case RC_FILL_RECT:
                    SDL_SetRenderDrawColor(renderer, 
                        (command.data.fill_rect.color) & 0xFF,          
                        (command.data.fill_rect.color >> 8) & 0xFF,     
                        (command.data.fill_rect.color >> 16) & 0xFF,   
                        (command.data.fill_rect.color >> 24) & 0xFF   
                    );
                    SDL_Rect test_rect = {command.data.fill_rect.x, command.data.fill_rect.y, command.data.fill_rect.w, command.data.fill_rect.h};  
                    SDL_RenderFillRect(renderer, &test_rect);
                break;
                case RC_DRAW_RECT:
                    SDL_SetRenderDrawColor(renderer, 
                        (command.data.fill_rect.color) & 0xFF,      
                        (command.data.fill_rect.color >> 8) & 0xFF,
                        (command.data.fill_rect.color >> 16) & 0xFF,
                        (command.data.fill_rect.color >> 24) & 0xFF
                    );
                    SDL_Rect draw_rect = {command.data.fill_rect.x, command.data.fill_rect.y, command.data.fill_rect.w, command.data.fill_rect.h}; 
                    SDL_RenderDrawRect(renderer, &draw_rect);
                break;
                case RC_DRAW_IMAGE: {
                    SDL_Rect image_rect = {command.data.draw_image.x, command.data.draw_image.y, command.data.draw_image.width, command.data.draw_image.height};  
                    SDL_RenderCopy(renderer, command.data.draw_image.image, NULL, &image_rect);
                }
                break;
                case RC_DRAW_CROPPED_IMAGE: {
                    SDL_Rect image_rect = {command.data.draw_image.x, command.data.draw_image.y, command.data.draw_image.crop_width, command.data.draw_image.crop_height};  
                    SDL_Rect from_rect = {command.data.draw_image.image_x, command.data.draw_image.image_y, command.data.draw_image.crop_width, command.data.draw_image.crop_height};
                    SDL_RenderCopy(renderer, command.data.draw_image.image, &from_rect, &image_rect);
                }
                break;
                case RC_DRAW_LINE: {
                    SDL_SetRenderDrawColor(renderer, 255,255,255,255);
                    SDL_RenderDrawLine(renderer, command.data.draw_line.x1, command.data.draw_line.y1, command.data.draw_line.x2, command.data.draw_line.y2);
                }
                break;
                }
        }
        // Clear the screen first
        /*void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);

        memcpy(pixels, game_state->output_buffer, screenWidth*screenHeight*4);
        SDL_UnlockTexture(texture);
        //SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        */
        SDL_RenderPresent(renderer);

    }

    free(game_memory.permanent_storage);
    free(game_memory.transient_storage);

    return 0;
}
