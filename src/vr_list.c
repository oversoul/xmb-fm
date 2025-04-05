#include "animation.h"
#include <stdio.h>
#include "vr_list.h"

static float item_y(const VerticalList *list, int i, size_t current) {
    float icon_spacing_vertical = list->icon_spacing_vertical;

    if (i < (int)current) {
        if (list->depth > 1)
            return icon_spacing_vertical * (i - (int)current + list->above_subitem_offset);
        return icon_spacing_vertical * (i - (int)current + list->above_item_offset);
    } else if (i == (int)current) {
        return icon_spacing_vertical * list->active_item_factor;
    }

    return icon_spacing_vertical * (i - (int)current + list->under_item_offset);
}

static void calculate_visible_range(const VerticalList *list, unsigned height, size_t list_size, unsigned current,
                                    unsigned *first, unsigned *last) {
    unsigned j;
    float base_y = list->margins_screen_top;

    *first = 0;
    *last = (unsigned)(list_size ? list_size - 1 : 0);

    if (current) {
        for (j = current; j-- > 0;) {
            float bottom = item_y(list, j, current) + base_y + list->icon_size;

            if (bottom < 0)
                break;

            *first = j;
        }
    }

    for (j = current + 1; j < list_size; j++) {
        float top = item_y(list, j, current) + base_y;

        if (top > height)
            break;

        *last = j;
    }
}

void update_vertical_list(VerticalList *list, float current_time) {
    unsigned i, end, height, entry_start, entry_end;
    int threshold = 0;
    size_t selection = list->selected;

    end = (unsigned)list->items_count;
    threshold = list->icon_size * 10;

    list->get_screen_size(NULL, &height);
    calculate_visible_range(list, height, end, (unsigned)selection, &entry_start, &entry_end);

    list->entry_start = entry_start;
    list->entry_end = entry_end + 1;

    for (i = list->entry_start; i < list->entry_end; i++) {
        float iy, real_iy;
        float ia = 0.5; // items_passive_alpha;
        float iz = 0.5; // items_passive_zoom;
        struct file_entry *node = list->items[i];

        iy = item_y(list, i, selection);
        real_iy = iy + list->margins_screen_top;

        if (i == selection) {
            ia = 1.0; // items_active_alpha;
            iz = 1.0; // items_active_zoom;
        }

        if (real_iy < -threshold || real_iy > height + threshold) {
            node->y = iy;
            node->zoom = iz;
            node->alpha = node->label_alpha = ia;
        } else {
            /* Move up/down animation */
            AnimatedProperty anim_entry;

            anim_entry.duration = 100;
            anim_entry.target = ia;
            anim_entry.subject = &node->alpha;
            anim_entry.start_time = current_time;

            gfx_animation_push(&anim_entry);

            anim_entry.subject = &node->label_alpha;
            anim_entry.start_time = current_time;

            gfx_animation_push(&anim_entry);

            anim_entry.target = iz;
            anim_entry.subject = &node->zoom;
            anim_entry.start_time = current_time;

            gfx_animation_push(&anim_entry);

            anim_entry.target = iy;
            anim_entry.subject = &node->y;
            anim_entry.start_time = current_time;

            gfx_animation_push(&anim_entry);
        }
    }
}
