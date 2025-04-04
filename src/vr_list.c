#include "animation.h"
#include "nanovg.h"

#include "vr_list.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void selection_pointer_changed(VerticalList *list) {
    unsigned i, end, height, entry_start, entry_end;
    int threshold = 0;
    size_t selection = list->selected;

    end = (unsigned)list->items_count;
    threshold = list->icon_size * 10;

    list->get_screen_size(NULL, &height);
    calculate_visible_range(list, height, end, (unsigned)selection, &entry_start, &entry_end);

    for (i = 0; i < end; i++) {
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
            anim_entry.start_time = glfwGetTime();

            gfx_animation_push(&anim_entry);

            anim_entry.subject = &node->label_alpha;
            anim_entry.start_time = glfwGetTime();

            gfx_animation_push(&anim_entry);

            anim_entry.target = iz;
            anim_entry.subject = &node->zoom;
            anim_entry.start_time = glfwGetTime();

            gfx_animation_push(&anim_entry);

            anim_entry.target = iy;
            anim_entry.subject = &node->y;
            anim_entry.start_time = glfwGetTime();

            gfx_animation_push(&anim_entry);
        }
    }
}

void draw_ui(const VerticalList *list, NVGcontext *vg) {
    unsigned end = (unsigned)list->items_count;
    unsigned height, entry_start, entry_end;
    list->get_screen_size(NULL, &height);
    calculate_visible_range(list, height, end, (unsigned)list->selected, &entry_start, &entry_end);

    for (int i = entry_start; i < entry_end; i++) {
        struct file_entry *node = list->items[i];

        float x = 190 + node->x;
        float y = list->margins_screen_top + node->y;

        NVGcolor icon_color = nvgRGBAf(0.8f, 0.8f, 1.0f, node->alpha);

        float size = list->icon_size;
        nvgFontSize(vg, size);
        nvgFontFace(vg, "icon");
        nvgFillColor(vg, icon_color);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        if (node->type == TYPE_DIRECTORY) {
            nvgText(vg, x - size / 2, y, "\ue950", NULL);
        } else if (node->type == TYPE_FILE) {
            nvgText(vg, x - size / 2, y, "\ue96d", NULL);
        }

        NVGcolor text_color = nvgRGBAf(1.0f, 1.0f, 1.0f, node->label_alpha);

        nvgFontSize(vg, 16);
        nvgFontFace(vg, "sans");
        nvgFillColor(vg, text_color);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(vg, x + 50, y, node->name, NULL);

        // Add glow effect for selected item
        if (i == list->selected) {
            float pulse = 0.5f + 0.3f * sinf(glfwGetTime() * 3.0f);
            NVGcolor glow_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.2f * pulse);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 45.0f, y - 20.0f, 400.0f, 40, 10.0f);
            nvgFillColor(vg, glow_color);
            nvgFill(vg);
        }
    }
}
