#include "game.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define ARRAY_INDEX(x, y, width) (x + width*y)
#define ARRAY_INDEX_TO_VEC2(index, width) ((vec2){index % width, index / width})


#define SCANCODE_1 30
#define SCANCODE_2 31
#define SCANCODE_3 32
#define SCANCODE_4 33
#define SCANCODE_5 34
#define SCANCODE_6 35
#define SCANCODE_7 36
#define SCANCODE_8 37
#define SCANCODE_9 38
#define SCANCODE_0 39
#define SCANCODE_A 4
#define SCANCODE_D 7
#define SCANCODE_S 22
#define SCANCODE_W 26
#define SCANCODE_TAB 43
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


static vec2 vec_minus(vec2 a, vec2 b) {
    return (vec2){a.x-b.x, a.y-b.y};
}

static vec2 vec_plus(vec2 a, vec2 b) {
    return (vec2){a.x+b.x, a.y+b.y};
}

static float vec_dot(vec2 v1, vec2 v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

static float vec_cross(vec2 a, vec2 b) {
    return a.x*b.y - a.y*b.x;
}

static float vec_length(vec2 v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

static float vec_length_sqr(vec2 v) {
    return v.x*v.x + v.y*v.y;
}

static vec2 vec_from_length_angle(float length, float angle) {
    return (vec2){cos(angle)*length, sin(angle)*length};
}

static vec2 vec_rotate_90_degrees(vec2 v) {
    return (vec2){-v.x, v.y};
}

static vec2 vec_rotate_minus_90_degrees(vec2 v) {
    return (vec2){v.y, -v.x};
}

static bool points_are_clockwise(vec2 p0, vec2 p1, vec2 p2) {
    return (vec_cross(vec_minus(p1, p0), vec_minus(p2, p1)) < 0);
}

static vec2 vec_normalize(vec2 v) {
    float length = vec_length(v);
    return (vec2){v.x/length, v.y/length};
}
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
static inline float clampf(float value, float min, float max) {
    return (value < min ? min: (value > max ? max : value));
}

static bool aabb_collision(float x1, float y1, float w1, float h1,
                          float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 &&
           x1 + w1 > x2 &&
           y1 < y2 + h2 &&
           y1 + h1 > y2;
}

static void render_command_push_draw_partial_image(RenderCommands* buffer, Image image, int image_x, int image_y, int partial_width, int partial_height, int x, int y) {
    RenderCommand* cmd = &buffer->buffer[buffer->count++];
    cmd->type = RC_DRAW_CROPPED_IMAGE;
    cmd->data.draw_image.x = x;
    cmd->data.draw_image.y = y;
    cmd->data.draw_image.image_x = image_x;
    cmd->data.draw_image.image_y = image_y;
    cmd->data.draw_image.crop_width = partial_width;
    cmd->data.draw_image.crop_height = partial_height;
    cmd->data.draw_image.width = image.width;
    cmd->data.draw_image.height = image.height;
    cmd->data.draw_image.image = image.image;
}

static void render_command_push_draw_rect(RenderCommands* buffer, int x, int y, int w, int h, u32 color) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_DRAW_RECT;
        cmd->data.fill_rect.x = x;
        cmd->data.fill_rect.y = y;
        cmd->data.fill_rect.w = w;
        cmd->data.fill_rect.h = h;
        cmd->data.fill_rect.color = color;
}

static void render_command_push_draw_image(RenderCommands* buffer, Image image, int x, int y) {
    RenderCommand* cmd = &buffer->buffer[buffer->count++];
    cmd->type = RC_DRAW_IMAGE;
    cmd->data.draw_image.x = x;
    cmd->data.draw_image.y = y;
    cmd->data.draw_image.width = image.width;
    cmd->data.draw_image.height = image.height;
    cmd->data.draw_image.image = image.image;
}

static void draw_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
        render_command_push_draw_rect(&state->render_command_buffer, _x, _y, width, height, color);
}

static void draw_cropped_image(GameState* state, u8 image_index, int image_x, int image_y, int from_width, int from_height, int _x, int _y, int image_width, int image_height) {
        render_command_push_draw_partial_image(&state->render_command_buffer, state->image_list[image_index], image_x, image_y, from_width, from_height, _x, _y);
}

static void render_command_push_fill_rect(RenderCommands* buffer, int x, int y, int w, int h, u32 color) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_FILL_RECT;
        cmd->data.fill_rect.x = x;
        cmd->data.fill_rect.y = y;
        cmd->data.fill_rect.w = w;
        cmd->data.fill_rect.h = h;
        cmd->data.fill_rect.color = color;
}

static void render_command_push_draw_line(RenderCommands* buffer, int x1, int y1, int x2, int y2, u32 color) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_DRAW_LINE;
        cmd->data.draw_line.x1 = x1;
        cmd->data.draw_line.y1 = y1;
        cmd->data.draw_line.x2 = x2;
        cmd->data.draw_line.y2 = y2;
        cmd->data.fill_rect.color = color;
}

static void render_command_push_clear(RenderCommands* buffer) {
        RenderCommand* cmd = &buffer->buffer[buffer->count++];
        cmd->type = RC_CLEAR;
}


static void fill_rect(GameState* state, int _x, int _y, int width, int height, u32 color) {
        render_command_push_fill_rect(&state->render_command_buffer, _x, _y, width, height, color);
}
