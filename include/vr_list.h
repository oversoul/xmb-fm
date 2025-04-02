#pragma once

#include "nanovg.h"

#include "fm.h"

typedef struct {
    int selected;
    bool toggled;

    /////
    int depth;
    float icon_spacing_vertical;

    int icon_size;
    float margins_screen_top;

    float above_subitem_offset;
    float above_item_offset;
    float active_item_factor;
    float under_item_offset;

    int items_count;
    struct file_entry **items;

    void (*get_screen_size)(unsigned *width, unsigned *height);
} VerticalList;

void selection_pointer_changed(VerticalList *list);
void draw_ui(const VerticalList *list, NVGcontext *vg);
