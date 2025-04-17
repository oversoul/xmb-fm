#pragma once

#include "hr_list.h"
#include "vr_list.h"

void draw_background(float width, float height, int theme);
void draw_folder_path(const HorizontalList *hr_list, const char *path);
void draw_vertical_list(const VerticalList *list);
void draw_horizontal_menu(const HorizontalList *hr_list, int x, int y);
void draw_text_preview(const char *text, float width, float height);
void draw_info(const VerticalList* vr_list, float width, float height);
