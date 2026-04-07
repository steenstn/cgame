#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
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

typedef struct wWindow {
    SDL_Window* window;
} wWindow;

typedef struct wRect {
    SDL_Rect rect;
} wRect;

typedef struct wScancode {
    SDL_Scancode scancode;
} wScancode;

typedef struct wRenderer {
    SDL_Renderer* renderer;
} wRenderer;

typedef struct wTexture {
    SDL_Texture* texture;
} wTexture;




typedef struct Image {
    int width;
    int height;
    SDL_Texture* texture;
} Image;


bool wInit() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize: %s\n", SDL_GetError());
        return false;
    }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image fail");
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);
    return true;
}

wWindow wCreateWindow(int width, int height) {
    SDL_Window* window = SDL_CreateWindow("SDL tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }
    return (wWindow) {window};
}

wRenderer wCreateRenderer(wWindow* window) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return (wRenderer) {renderer};
}

Image loadImage(wRenderer* renderer, char* path, int width, int height) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer->renderer, loadedSurface);
    Image image =  (Image){.width = width, .height = height, .texture = texture};
    SDL_FreeSurface(loadedSurface);
    return image;
}

int wRenderCopy(wRenderer renderer, wTexture texture, wRect* srcRect, wRect* dstRect) {
 return SDL_RenderCopy(renderer.renderer, texture.texture, &srcRect->rect, &dstRect->rect);
}

void wSetRenderDrawColor(wRenderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(renderer->renderer, r, g, b, a);
}

void wRenderClear(wRenderer* renderer) {
    SDL_RenderClear(renderer->renderer);
}

void wFillRect(wRenderer* renderer, int x, int y, int width, int height) {
    SDL_Rect rect = {.x = x, .y = y, .w = width, .h = height};
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void wDrawRect(wRenderer* renderer, int x, int y, int width, int height) {
    SDL_Rect rect = {.x = x, .y = y, .w = width, .h = height};
    SDL_RenderDrawRect(renderer->renderer, &rect);
}

void wDrawImage(wRenderer* renderer, Image* image, int x, int y) {
    SDL_Rect rect = {.x = x, .y = y, .w = image->width, .h = image->height};
    SDL_RenderCopy(renderer->renderer,  image->texture, NULL, &rect);
}

void wRenderFrame(wRenderer* renderer) {
    SDL_RenderPresent(renderer->renderer);
}


