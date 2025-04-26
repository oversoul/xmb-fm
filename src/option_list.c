#include "option_list.h"
#include "animation.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void option_list_event_handler(EventType event, OptionList *list, void *data) {
    switch (event) {
    case OPTION_EVENT_OPEN_MENU:
        list->depth = 1;
        update_option_list(list);
        break;

    case OPTION_EVENT_CLOSE_MENU:
        if (list->depth <= 0)
            return;

        list->depth--;
        list->current->selected = 0;
        if (list->current->parent)
            list->current = list->current->parent;
        update_option_list(list);
        break;

    case OPTION_EVENT_MOVE_SELECTION: {
        int direction = *(int *)data; // +1 for down, -1 for up

        if (direction < 0 && list->current->selected > 0)
            list->current->selected--;
        else if (direction > 0 && list->current->selected < list->current->items_count - 1)
            list->current->selected++;

        update_option_list(list);
        break;
    }

    case OPTION_EVENT_SELECT_ITEM: {
        Option *current = &list->current->items[list->current->selected];

        if (list->on_item_selected == NULL)
            return;

        list->on_item_selected(current);
        if (list->current->parent)
            list->current = list->current->parent;
        list->current->selected = 0;
        list->depth--;
        update_option_list(list);
        break;
    }

    case OPTION_EVENT_OPEN_SUBMENU: {
        Option *current = &list->current->items[list->current->selected];

        if (current->submenu && current->submenu->items_count > 0) {
            list->depth++;
            list->current = current->submenu;
            update_option_list(list);
        }
        break;
    }
    default:
        break;
    }
}

void update_option_list(OptionList *op_list) {
    // if (list->items_count == 0)
    //     return;

    Options *list = op_list->current;

    animation_push(0.1, op_list->depth * -(float)OPTION_LIST_WIDTH, &op_list->x, OptionListTag);

    uint32_t height = 0;
    uint32_t end = list->items_count;
    if (op_list->get_screen_size != NULL) {
        op_list->get_screen_size(NULL, &height);
    }

    for (size_t i = 0; i < end; i++) {
        Option node = list->items[i];

        node.y = 40 * i;
    }
}
