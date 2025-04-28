#pragma once

#include "input.h"
#include "hr_list.h"
#include "vr_list.h"
#include "option_list.h"

typedef struct {
    int theme;
    int width;
    int height;
    bool show_info;
    bool show_preview;
    char buffer[512];
} DrawState;

void draw_folder_path(const HorizontalList *hr_list, const char *path, float x, float y);
void draw_vertical_list(const VerticalList *list, float x);
void draw_horizontal_menu(const HorizontalList *hr_list, int x, int y);
void draw_text_preview(DrawState *state);
void draw_info(const VerticalList *vr_list, float width, float height);
void draw_option_list(OptionList *op_list, DrawState *state);
void draw_input_field(Input *input, const char *title, DrawState *state);
