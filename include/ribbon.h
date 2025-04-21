#pragma once

#include <stdint.h>

typedef struct {
    uint32_t ribbon_program;
    uint32_t ribbon_vertex_array;
    uint32_t ribbon_vertex_buffer;
} RibbonState;

void init_ribbon();
void draw_ribbon(float width, float height, float time);
