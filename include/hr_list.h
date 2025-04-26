#pragma once

#include "signal.h"

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
void update_horizontal_list(HorizontalList *hr_list);
void horizontal_list_event_handler(EventType type, void *context, void *data);
