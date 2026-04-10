#include "game.h"
#include <stdio.h>
#include <stdlib.h>

static GameState *init() {

GameState* gameState = malloc(sizeof(GameState));
    printf("Initializing game\n");
    gameState->things = NULL;
    gameState->screenWidth = 1600;
    gameState->screenHeight = 1200;
    gameState->mouseX = 0;
    gameState->mouseY = 0;
    return gameState;
}

static bool step(GameState* state, wRenderer* renderer) {
    state->r = 40;
    state->g = 250;
    state->b = 40;
    return true;
}

static GameAPI api = {
    .init = init,
    .step = step
};

GameAPI* get_game_api() {
    return &api;
}


