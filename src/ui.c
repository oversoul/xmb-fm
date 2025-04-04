#include "ui.h"
#include "hr_list.h"
#include "vr_list.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdlib.h>

void draw_background(NVGcontext *vg, float width, float height) {
    // Simple blue gradient background
    NVGpaint bg_paint = nvgLinearGradient(vg, 0, 0, 0, height, nvgRGBA(0, 20, 50, 255), nvgRGBA(0, 10, 30, 255));
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, width, height);
    nvgFillPaint(vg, bg_paint);
    nvgFill(vg);

    // Add animated wave effect
    float time = glfwGetTime();
    for (int i = 0; i < 5; i++) {
        float alpha = 0.12f - i * 0.02f;
        float offset = i * 80.0f;
        float amplitude = 25.0f - i * 4.0f;
        float frequency = 0.006f + i * 0.001f;
        float speed = 0.7f + i * 0.1f;

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, height * 0.7f + sinf(time * speed) * amplitude + offset);

        for (int x = 0; x <= width; x += 15) {
            float y = height * 0.7f + sinf(time * speed + x * frequency) * amplitude + offset;
            nvgLineTo(vg, x, y);
        }

        nvgLineTo(vg, width, height);
        nvgLineTo(vg, 0, height);
        nvgClosePath(vg);

        NVGcolor wave_color = nvgRGBAf(0.2f, 0.6f, 1.0f, alpha);
        nvgFillColor(vg, wave_color);
        nvgFill(vg);
    }
}

void draw_selected_item_title(NVGcontext *vg, const HorizontalList *hr_list) {
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

void draw_horizontal_menu(NVGcontext *vg, const HorizontalList *hr_list, int x, int y) {
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

void draw_folder_path(NVGcontext *vg, const HorizontalList *hr_list, const char *path) {
    if (hr_list->depth > 0) {
        float x = 200;
        nvgFontSize(vg, 12);
        nvgFontFace(vg, "sans");
        nvgFillColor(vg, nvgRGB(255, 255, 255));
        nvgText(vg, x, 160, path, NULL);

        // for (size_t i = 0; i < fm->history_pos; ++i) {
        //     float bounds[4];
        //     nvgTextBounds(vg, 0, 0, fm->history[i]->name, NULL, bounds);
        //     printf("BOUNDS: %f\n", bounds[2]);
        //     nvgText(vg, x, 160, fm->history[i]->name, NULL);
        //     x += bounds[2] + 10;
        // }
    }
}

void draw_vertical_list(NVGcontext *vg, const VerticalList *list) {
    for (int i = list->entry_start; i < list->entry_end; i++) {
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
