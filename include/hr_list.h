#pragma once

typedef struct {
    char title[50];
    char path[50];
    char icon[10];
} HrItem;

typedef struct {
    int depth;
    int selected;
    float scroll;
    HrItem *items;
    int items_count;
} HorizontalList;

void init_horizontal_list(HorizontalList *hr_list);
void update_horizontal_list(HorizontalList *hr_list, float current_time);
