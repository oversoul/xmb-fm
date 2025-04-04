#pragma once

#include "nanovg.h"

typedef struct {
    char title[50];
    char path[50];
    char icon[100];
} HrItem;

typedef struct {
    int depth;
    int selected;
    int items_count;
    HrItem *items;
    float scroll;
} HorizontalList;

void init_horizontal_list(HorizontalList *hr_list);
void update_horizontal_list(HorizontalList *hr_list);
void draw_horizontal_menu(NVGcontext *vg, HorizontalList *hr_list, int x, int y);
