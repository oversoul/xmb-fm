#include "option_list.h"
#include "animation.h"
#include <GLFW/glfw3.h>
#include <stddef.h>
#include <stdint.h>

bool handle_option_list_key(OptionList *op_list, int key, float current_time) {
    if (op_list->is_open) {
        switch (key) {
        case GLFW_KEY_I:
        case GLFW_KEY_ESCAPE:
            op_list->selected = 0;
            op_list->is_open = false;
            update_option_list(op_list, current_time);
            return true;
        case GLFW_KEY_UP:
            if (op_list->selected > 0)
                op_list->selected--;
            return true;
        case GLFW_KEY_DOWN:
            if (op_list->selected < op_list->items_count - 1)
                op_list->selected++;
            return true;
        case GLFW_KEY_ENTER:
            if (op_list->on_item_selected != NULL) {
                op_list->on_item_selected(&op_list->items[op_list->selected]);
                op_list->selected = 0;
                op_list->is_open = false;
                update_option_list(op_list, current_time);
            }

            return true;
        }
    }

    if (key == GLFW_KEY_I) {
        op_list->is_open = true;
        update_option_list(op_list, current_time);
        return true;
    }

    return false;
}

void update_option_list(OptionList *list, float current_time) {
    // if (list->items_count == 0)
    //     return;

    AnimatedProperty anim;
    anim.duration = 0.2;
    anim.start_time = current_time;

    anim.target = list->is_open ? -400 : 0;

    anim.subject = &list->x;
    gfx_animation_push(&anim, OptionListTag);

    uint32_t height = 0;
    uint32_t end = list->items_count;
    if (list->get_screen_size != NULL) {
        list->get_screen_size(NULL, &height);
    }

    for (size_t i = 0; i < end; i++) {
        Option node = list->items[i];

        node.y = 40 * i;
    }
}
