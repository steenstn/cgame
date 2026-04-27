static u8 level_get_tile(Level* level, int x, int y) {

    int index = ARRAY_INDEX((int)((x)/level->tile_size), (int)(y/level->tile_size), level->level_width);

    return level->tiles[index];
}
