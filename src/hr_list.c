#include "hr_list.h"
#include "animation.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

void init_horizontal_list(HorizontalList *hr_list) {}

void update_horizontal_list(HorizontalList *hr_list) {
    float offset = 150;

    AnimatedProperty anim;
    anim.duration = 100;
    anim.start_time = glfwGetTime();
    anim.target = hr_list->selected * offset;
    if (hr_list->depth > 0) {
        anim.target += 50;
    }

    anim.subject = &hr_list->scroll;

    gfx_animation_push(&anim);
}

void draw_selected_item_title(NVGcontext *vg, HorizontalList *hr_list) {
    NVGcolor text_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.9f);

    nvgFontSize(vg, 24);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, text_color);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(vg, 30, 40, hr_list->items[hr_list->selected].title, NULL);

    // subtitle
    nvgFontSize(vg, 16);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, text_color);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(vg, 30, 70, hr_list->items[hr_list->selected].path, NULL);
}

void draw_horizontal_menu(NVGcontext *vg, HorizontalList *hr_list, int x, int y) {
    float gap = 150.0f;
    float base_x = x - hr_list->scroll;

    // draw title
    draw_selected_item_title(vg, hr_list);
    float size = 50;

    NVGcolor icon_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0);
    if (hr_list->depth > 0) {
        nvgBeginPath(vg);

        float scale_factor = 1.5f;
        size *= scale_factor;
        float x = base_x + (hr_list->selected * gap);
        nvgRoundedRect(vg, x - size / 2, y - size / 2, size, size, 20);
        nvgFillColor(vg, icon_color);
        nvgFill(vg);
        return;
    }

    for (int i = 0; i < hr_list->items_count; i++) {
        float size = 50;
        float x = base_x + (i * gap);

        // Calculate dynamic scale based on proximity to selected item
        float distance = abs(i - hr_list->selected);
        float scale_factor = 1.0f;
        if (distance == 0) {
            scale_factor = 1.5f;
        }

        float opacity = 1.0;
        size *= scale_factor;

        // Draw category icon (simplified as circle)
        NVGcolor icon_color = nvgRGBAf(1.0f, 1.0f, 1.0f, opacity);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x - size / 2, y - size / 2, size, size, 20);
        nvgFillColor(vg, icon_color);
        nvgFill(vg);

        // icon
        nvgFontSize(vg, 30 * scale_factor);
        nvgFillColor(vg, nvgRGBAf(0, 0, 0, opacity));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, x, y + scale_factor * 5, hr_list->items[i].icon, NULL);
    }
}
