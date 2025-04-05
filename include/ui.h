#pragma once

#include "nanovg.h"
#include "hr_list.h"
#include "vr_list.h"

void draw_background(NVGcontext *vg, float width, float height);
void draw_folder_path(NVGcontext *vg, const HorizontalList *hr_list, const char *path);
void draw_vertical_list(NVGcontext *vg, const VerticalList *list);
void draw_horizontal_menu(NVGcontext *vg, const HorizontalList *hr_list, int x, int y);
void draw_text_preview(NVGcontext *vg, const char *text, size_t len, float width, float height);
