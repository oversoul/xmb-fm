#include "hr_list.h"
#include <stdio.h>
#include <stdlib.h>

void init_horizontal_list(HorizontalList *hr_list) {
    hr_list->anim.offset = 0;
    hr_list->anim.target_offset = 0;
}

void update_horizontal_list(HorizontalList *hr_list, float anim_factor) {
    float direction = (hr_list->anim.target_offset > hr_list->anim.offset) ? 1.0f : -1.0f;
    hr_list->anim.offset += direction * anim_factor * 20;
    hr_list->anim.offset = direction * hr_list->anim.offset > direction * hr_list->anim.target_offset
                               ? hr_list->anim.target_offset
                               : hr_list->anim.offset;
}

void draw_selected_item_title(NVGcontext *vg, HorizontalList *hr_list) {
    // Draw category title if selected or close to selected
    NVGcolor text_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.9f);

    nvgFontSize(vg, 24);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, text_color);
    nvgText(vg, 30, 40, hr_list->items[hr_list->selected].title, NULL);

    // subtitle
    nvgFontSize(vg, 16);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, text_color);
    nvgText(vg, 30, 70, hr_list->items[hr_list->selected].path, NULL);
}

void draw_horizontal_menu(NVGcontext *vg, HorizontalList *hr_list, int x, int y) {
    float gap = 150.0f;
    float base_x = x - hr_list->anim.offset; // Animated horizontal offset

    // draw title
    draw_selected_item_title(vg, hr_list);

    for (int i = 0; i < hr_list->category_count; i++) {
        float x = base_x + (i * gap);

        // Calculate dynamic scale based on proximity to selected item
        float distance = abs(i - hr_list->selected);
        float scale_factor = 1.0f;
        if (distance == 0) {
            scale_factor = 1.5f;
        }

        float opacity = 1.0;

        // Draw category icon (simplified as circle)
        NVGcolor icon_color = nvgRGBAf(1.0f, 1.0f, 1.0f, opacity);

        float size = 50 * scale_factor;

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x - size / 2, y - size / 2, size, size, 20);
        nvgFillColor(vg, icon_color);
        nvgFill(vg);
    }
}
