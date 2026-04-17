.PHONY: game test
all: lsp game
	gcc -std=c99 -g -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra main.c -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -o game

lsp:
	@echo "# Auto-generated - run make to regenerate" > .clangd
	@echo "CompileFlags:" >> .clangd
	@echo "  Add: [-std=c99, -Wall, -I.]" >> .clangd
	@echo "  Compiler: gcc" >> .clangd
	@echo "Scanning unity build structure..." >&2
	@awk 'BEGIN { print "# Files included via game.h:" > "/dev/stderr" } \
		/^#include "[^"]+\.c"/ { \
			gsub(/"/, "", $$2); \
			print "  - " $$2 > "/dev/stderr"; \
		}' game.h
	@awk 'BEGIN { print "# Files included via game.c:" > "/dev/stderr" } \
		/^#include "[^"]+\.c"/ { \
			gsub(/"/, "", $$2); \
			print "  - " $$2 > "/dev/stderr"; \
		}' game.c
	@awk 'BEGIN { n = 0 } \
		FNR==NR && /^#include "[^"]+\.c"/ { \
			gsub(/"/, "", $$2); \
			h_files[++h_n] = $$2; \
			next; \
		} \
		FNR!=NR && /^#include "[^"]+\.c"/ { \
			gsub(/"/, "", $$2); \
			c_files[++c_n] = $$2; \
			next; \
		} \
		END { \
			for (i = 1; i <= h_n; i++) { \
				all[++n] = h_files[i]; \
			} \
			for (i = 1; i <= c_n; i++) { \
				all[++n] = c_files[i]; \
			} \
			for (i = 1; i <= n; i++) { \
				print "---"; \
				print "If:"; \
				f = all[i]; \
				gsub(/\./, "\\.", f); \
				print "  PathMatch: " f; \
				print "CompileFlags:"; \
				if (i == 1) { \
					print "  Add: []"; \
				} else { \
					printf "  Add: ["; \
					if (i > h_n) { \
						printf "-include, SDL2/SDL_keycode.h, -include, SDL2/SDL_scancode.h, "; \
					} \
					printf "-include, game.h"; \
					for (j = 1; j < i; j++) { \
						printf ", -include, %s", all[j]; \
					} \
					print "]"; \
				} \
			} \
		}' game.h game.c >> .clangd
	@echo "Generated .clangd with $(shell grep -c 'PathMatch' .clangd) file entries"
game:
	gcc -fPIC -shared -o libgame.so -lSDL2 -lm game.c
test:
	gcc test.c -o test
