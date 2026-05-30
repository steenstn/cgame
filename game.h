#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vec2 {
    float x,y;
} vec2;

typedef struct vec2_i {
    int x, y;
} vec2_i;

bool vec2_i_equals(vec2_i a, vec2_i b) {
    return a.x == b.x && a.y == b.y;
}


#define MAX_THINGS 500
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400

uint8_t walls[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g'};

typedef struct DictEntry {
    vec2_i key;
    vec2_i value;
} DictEntry;

typedef struct Dict {
    DictEntry* entries;
    int capacity;
    int size;
} Dict;
typedef struct Arena {
    uint8_t* base;
    size_t size;
    size_t used;
} Arena;

typedef struct Queue {
    int capacity;
    int size;
    int head;
    int tail;
    int* entries;
} Queue;

typedef struct QueueVec2 {
    int capacity;
    int size;
    int head;
    int tail;
    vec2_i* entries;
} QueueVec2;
typedef struct IntList {
    int size;
    int* entries;
} IntList;

typedef struct Vec2List {
    int size;
    vec2* entries;
} Vec2List;

typedef struct Vec2_i_List {
    int size;
    vec2_i* entries;
} Vec2_i_List;


typedef struct Image {
    void* image;
    int width, height;
} Image;

typedef struct PlatformAPI {
    bool (*read_whole_file)(char* path, void* result, size_t length);
    bool (*write_file)(char* path, void* data, size_t length);
    Image (*load_image)(char* path);
} PlatformAPI;

bool platform_read_whole_file(char* path, void* result, size_t length);

typedef struct GameMemory {
    bool is_initialized;
    void* permanent_storage;
    size_t permanent_storage_size;
    void* transient_storage;
    size_t transient_storage_size;
    PlatformAPI platform_api;
} GameMemory;

typedef enum Direction {
    LEFT = 0, 
    RIGHT = 1
} Direction;

typedef struct Thing {
    uint64_t flags;
    float x,y, old_x, old_y;
    float vx, vy;
    float ax, ay;
    int width, height;
    int projectile_counter;
    int next_thing;
    bool jumping;
    bool moving;
    Vec2_i_List path;
    Direction direction;
    int animation_counter;
    int animation_frame;
} Thing;

typedef struct Triangle {
    vec2 p[3];
    vec2 normals[3];
} Triangle;

typedef struct MouseState {
    int x,y;
    bool left_button_down;
    bool right_button_down;
    bool left_button_click;
    bool right_button_click;
} MouseState;

typedef struct KeyboardState {
    uint8_t* keys_down;
    uint8_t* keys_hit;
} KeyboardState;

typedef enum Mode {
    PLAY,
    EDITOR
} Mode;
 

typedef enum {
    RC_CLEAR,
    RC_FILL_RECT,
    RC_DRAW_RECT,
    RC_DRAW_IMAGE,
    RC_DRAW_CROPPED_IMAGE,
    RC_DRAW_LINE,
} RenderCommandType;

typedef struct {
    RenderCommandType type;
    union {
        struct {int x, y, w, h; uint32_t color;} fill_rect;
        struct {int x1, y1, x2, y2; uint32_t color;} draw_line;
        struct {void* image; int index, x, y, width, height, image_x, image_y, crop_width, crop_height;} draw_image;
    } data;
} RenderCommand;

typedef struct RenderCommands {
    int count;
    int capacity;
    RenderCommand* buffer;
} RenderCommands;







typedef enum ActiveTool {
    TOOL_PLACE_WALL,
    TOOL_ERASE_WALL,
    TOOL_SELECT,
} ActiveTool;

typedef struct EditorState {
    ActiveTool active_tool;
    bool thing_selected;
    int selected_thing_index;
    int mouse_position_level_index;
} EditorState;


typedef struct Level {
    uint8_t* tiles;
    uint8_t* level_visibility;
    int tile_size;
    int level_width;
    int level_height;
} Level;

typedef struct GameState {
    Arena permanent_arena;
    Arena frame_arena;
    Arena scratch_arena;

    uint64_t ms_elapsed;
    Thing* things;
    Level level;
    int screenWidth;
    int screenHeight;
    int viewportX;
    int viewportY;
    Image* image_list;
    Triangle* triangles;

    KeyboardState keyboard_state;
    MouseState mouse_state;
    Mode mode;

    EditorState editor_state;

    PlatformAPI platform_api;

    RenderCommands render_command_buffer;
} GameState;


typedef struct GameAPI {
    GameState *(*init)(GameMemory* gameMemory);
    bool (*update_and_render)(GameState* state, const uint8_t* key_states, uint64_t ms_elapsed);
} GameAPI;

GameAPI* get_game_api();


#endif 
