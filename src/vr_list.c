#include "animation.h"
#include <stdio.h>
#include "vr_list.h"

void init_vertical_list(VerticalList *vr_list) {
    vr_list->above_subitem_offset = 0.0f;
    vr_list->above_item_offset = -1.5f;
    vr_list->active_item_factor = 1.0f;
    vr_list->under_item_offset = 1.0f;

    vr_list->icon_size = 28.0;
    vr_list->margins_screen_top = 200;
    vr_list->icon_spacing_vertical = 50.0;
}

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

static void find_visible_items(const VerticalList *list, uint32_t screen_height, uint32_t total_items,
                               uint32_t selected_index, uint32_t *start_index, uint32_t *end_index) {
    float y_offset = list->margins_screen_top;

    *start_index = 0;
    *end_index = total_items > 0 ? (total_items - 1) : 0;

    unsigned i;
    // Look upward from the current index
    for (i = selected_index; i-- > 0;) {
        float item_bottom = item_y(list, i, selected_index) + y_offset + list->icon_size;
        if (item_bottom < 0)
            break;
        *start_index = i;
    }

    // Look downward from the current index
    for (i = selected_index + 1; i < total_items; ++i) {
        float item_top = item_y(list, i, selected_index) + y_offset;
        if (item_top > screen_height)
            break;
        *end_index = i;
    }
}

void update_vertical_list(VerticalList *list, float current_time) {
    int threshold = 0;
    size_t selection = list->selected;

    if (list->items_count == 0)
        return;

    uint32_t end = (uint32_t)list->items_count;
    threshold = list->icon_size * 10;

    uint32_t height, entry_start, entry_end;
    list->get_screen_size(NULL, &height);
    find_visible_items(list, height, end, selection, &entry_start, &entry_end);

    list->entry_start = entry_start;
    list->entry_end = entry_end + 1;

    float default_zoom = 0.6;
    float default_alpha = 0.6;

    for (size_t i = 0; i < end; i++) {
        float y_pos, real_y_pos;
        float zoom = default_zoom;
        float label_alpha = default_alpha;
        struct file_entry *node = list->items[i];

        y_pos = item_y(list, i, selection);
        real_y_pos = y_pos + list->margins_screen_top;

        if (i == selection) {
            label_alpha = 1.0;
            zoom = 1.0;
        }

        if (real_y_pos < -threshold || real_y_pos > height + threshold) {
            node->y = y_pos;
            node->zoom = zoom;
            node->alpha = node->label_alpha = label_alpha;
        } else {
            animation_push(0.2, current_time, y_pos, &node->y, VerticalListTag);
            animation_push(0.05, current_time, zoom, &node->zoom, VerticalListTag);
            animation_push(0.2, current_time, label_alpha, &node->alpha, VerticalListTag);
            animation_push(0.2, current_time, label_alpha, &node->label_alpha, VerticalListTag);

            // printf("IY: %f\n", iy);
            // node->y = iy;
            // node->zoom = iz;
            // node->alpha = node->label_alpha = ia;
        }
    }
}
