all: lsp
	gcc -std=c99 -Wall -Wextra main.c -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o game

lsp:
	@awk 'BEGIN { n=0 } \
	/^#include "[^"]+\.c"/ { \
		gsub(/"/, "", $$2); \
		files[++n] = $$2; \
	} END { \
		print "# Auto-generated - run make to regenerate"; \
		print "CompileFlags:"; \
		print "  Add: [-std=c99, -Wall, -I.]"; \
		print "  Compiler: gcc"; \
		for (i = 2; i <= n; i++) { \
			print "---"; \
			print "If:"; \
			f = files[i]; gsub(/\./, "\\.", f); \
			print "  PathMatch: " f; \
			print "CompileFlags:"; \
			printf "  Add: ["; \
			for (j = 1; j < i; j++) { \
				if (j > 1) printf ", "; \
				printf "-include, %s", files[j]; \
			} \
			print "]"; \
		} \
	}' main.c > .clangd
	@echo "Generated .clangd"
