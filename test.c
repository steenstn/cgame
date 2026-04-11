
#include "game_engine.h"
#include <stdio.h>
int main() {

    u64 flags = 1 | 2 | 4;
    printf("Before: %lu\n", flags);
    flags = flags_is_set(flags, 2);
    printf("2: %lu\n", flags);
    flags = flags_is_set(flags, 3);
    printf("After again: %lu\n", flags);
    flags = flags_is_set(flags, 8);
    printf("After yetagain: %lu\n", flags);

return 0;
}
