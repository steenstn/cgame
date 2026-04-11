
#include "game_engine.h"
#include <stdio.h>
int main() {

    u64 flags = 1 | 2 | 4;
    printf("Before: %lu\n", flags);
    flags = flags_flip(flags, 2);
    printf("After: %lu\n", flags);
    flags = flags_flip(flags, 2);
    printf("After again: %lu\n", flags);
    flags = flags_flip(flags, 2);
    printf("After yetagain: %lu\n", flags);

return 0;
}
