#include <stdbool.h>
#include <stdint.h>
#define ARRAY_INDEX(x, y, width) (x + width*y)

#define SCANCODE_A 4
#define SCANCODE_D 7
#define SCANCODE_S 22
#define SCANCODE_W 26
#define SCANCODE_LSHIFT 225

enum KeyCode {
    KEY_UP,
    KEY_LEFT,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_SHIFT,
    _NUM_KEY_CODES,
};

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;


static inline bool flags_is_set(u64 flags, u64 flag_to_check) {
    return (flags & flag_to_check) == flag_to_check;
}

static inline u64 flags_set(u64 flags, u64 flag_to_set) {
    return flags | flag_to_set;
}

static inline u64 flags_unset(u64 flags, u64 flag_to_unset) {
    return flags & (flags ^ flag_to_unset);
}

static inline u64 flags_flip(u64 flags, u64 flag_to_flip) {
    return flags ^ flag_to_flip;
}

static inline int clamp(int value, int min, int max) {
    return (value < min ? min: (value > max ? max : value));
}
