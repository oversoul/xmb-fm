#pragma once

typedef struct {
    char title[60];
    float y;
} Option;

typedef struct {
    float x;
    int selected;
    bool is_open;

    Option *items;
    int items_count;

    void (*get_screen_size)(unsigned *width, unsigned *height);
} OptionList;

void update_option_list(OptionList *list, float current_time);
