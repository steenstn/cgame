
#include "game_engine.h"
#include <stdio.h>
int main() {

    printf("expected %d: %d\n", 5,clamp(5, 2,6));
    printf("expected %d: %d\n", 6, clamp(5, 6,6));
    printf("expected %d: %d\n", 4, clamp(5, 2,4));

return 0;
}
