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



