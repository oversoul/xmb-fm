#pragma once

#include <stdint.h>

typedef struct {
    uint32_t ribbon_program;
    uint32_t ribbon_vertex_array;
    uint32_t ribbon_vertex_buffer;

    uint32_t bg_vao;
    uint32_t bg_vbo;
    uint32_t bg_program;
} RibbonState;

void init_ribbon();
void draw_ribbon(float width, float height, float time);
void draw_background(float width, float height, int theme);
