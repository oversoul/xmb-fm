#pragma once

#include "signal.h"
#include <stddef.h>

#define OPTION_LIST_WIDTH 300

struct Options;

typedef struct option {
    float y;
    char title[60];
    struct Options *submenu;
} Option;

typedef struct Options {
    int selected;

    Option *items;
    int items_count;

    struct Options *parent;
} Options;

typedef struct {
    float x;
    size_t depth;
    Options *root;
    Options *current;

    void (*get_screen_size)(unsigned *width, unsigned *height);
    void (*on_item_selected)(Option *option);
} OptionList;

void update_option_list(OptionList *list);
void option_list_event_handler(EventType event, OptionList *list, void *data);
