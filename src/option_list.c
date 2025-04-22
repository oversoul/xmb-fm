#include "option_list.h"
#include "animation.h"
#include <stddef.h>
#include <stdint.h>

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
