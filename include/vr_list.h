#pragma once

#include "fm.h"
#include "signal.h"

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

    unsigned entry_start;
    unsigned entry_end;

    void (*get_screen_size)(unsigned *width, unsigned *height);
} VerticalList;

void init_vertical_list(VerticalList *list);
void update_vertical_list(VerticalList *list);
void vertical_list_event_handler(EventType type, void *context, void *data);
