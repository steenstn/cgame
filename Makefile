all:
	gcc -std=c99 -Wall -Wextra main.c -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o game
