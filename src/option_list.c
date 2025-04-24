#include "option_list.h"
#include "animation.h"
#include <GLFW/glfw3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

bool handle_option_list_key(OptionList *list, int key, float current_time) {
    if (list->depth > 0) {
        switch (key) {
        case GLFW_KEY_I:
        case GLFW_KEY_ESCAPE:
            list->depth--;
            list->current->selected = 0;
            if (list->current->parent)
                list->current = list->current->parent;
            update_option_list(list, current_time);
            return true;
        case GLFW_KEY_UP:
            if (list->current->selected > 0)
                list->current->selected--;
            return true;
        case GLFW_KEY_DOWN:
            if (list->current->selected < list->current->items_count - 1)
                list->current->selected++;
            return true;
        case GLFW_KEY_ENTER:
            Option *current = &list->current->items[list->current->selected];
            if (current->submenu && current->submenu->items_count > 0) {
                list->depth++;
                list->current = current->submenu;
                update_option_list(list, current_time);
                return true;
            }

            if (list->on_item_selected != NULL) {
                list->on_item_selected(current);
                if (list->current->parent)
                    list->current = list->current->parent;
                list->current->selected = 0;
                list->depth--;
                update_option_list(list, current_time);
            }

            return true;
        }
        return true;
    }

    if (key == GLFW_KEY_I) {
        list->depth = 1;
        update_option_list(list, current_time);
        return true;
    }

    return false;
}

void update_option_list(OptionList *op_list, float current_time) {
    // if (list->items_count == 0)
    //     return;

    Options *list = op_list->current;

    animation_push(0.1, current_time, op_list->depth * -(float)OPTION_LIST_WIDTH, &op_list->x, OptionListTag);

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
