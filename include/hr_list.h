#pragma once

#include "nanovg.h"

typedef struct {
    char title[50];
    char path[50];
    char icon[100];

} HrItem;

typedef struct {

    int selected;
    int category_count;
    HrItem *items;
    struct {
        float offset;        // Current horizontal offset
        float target_offset; // Target horizontal offset
    } anim;

} HorizontalList;

void init_horizontal_list(HorizontalList *hr_list);
void update_horizontal_list(HorizontalList *hr_list, float anim_factor);
void draw_horizontal_menu(NVGcontext *vg, HorizontalList *hr_list, int x, int y);
