#include "draw.h"
#include "ui.h"
#include "hr_list.h"
#include "vr_list.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void draw_background(float width, float height, int theme) {
    static Color gradient_golden[4] = {
        {174 / 255.0, 123 / 255.0, 44 / 255.0, 1.0},
        {205 / 255.0, 174 / 255.0, 84 / 255.0, 1.0},
        {58 / 255.0, 43 / 255.0, 24 / 255.0, 1.0},
        {58 / 255.0, 43 / 255.0, 24 / 255.0, 1.0},
    };
    static Color gradient_legacy_red[4] = {
        {171 / 255.0, 70 / 255.0, 59 / 255.0, 1.0},
        {171 / 255.0, 70 / 255.0, 59 / 255.0, 1.0},
        {190 / 255.0, 80 / 255.0, 69 / 255.0, 1.0},
        {190 / 255.0, 80 / 255.0, 69 / 255.0, 1.0},
    };
    static Color gradient_electric_blue[4] = {
        {1 / 255.0, 2 / 255.0, 67 / 255.0, 1.0},
        {1 / 255.0, 73 / 255.0, 183 / 255.0, 1.0},
        {1 / 255.0, 93 / 255.0, 194 / 255.0, 1.0},
        {3 / 255.0, 162 / 255.0, 254 / 255.0, 1.0},
    };
    static Color gradient_dark_purple[4] = {
        {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
        {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
        {92 / 255.0, 44 / 255.0, 92 / 255.0, 1.0},
        {148 / 255.0, 90 / 255.0, 148 / 255.0, 1.0},
    };
    static Color gradient_midnight_blue[4] = {
        {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
        {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
        {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
        {44 / 255.0, 62 / 255.0, 80 / 255.0, 1.0},
    };
    static Color gradient_apple_green[4] = {
        {102 / 255.0, 134 / 255.0, 58 / 255.0, 1.0},
        {122 / 255.0, 131 / 255.0, 52 / 255.0, 1.0},
        {82 / 255.0, 101 / 255.0, 35 / 255.0, 1.0},
        {63 / 255.0, 95 / 255.0, 30 / 255.0, 1.0},
    };
    static Color gradient_undersea[4] = {
        {23 / 255.0, 18 / 255.0, 41 / 255.0, 1.0},
        {30 / 255.0, 72 / 255.0, 114 / 255.0, 1.0},
        {52 / 255.0, 88 / 255.0, 110 / 255.0, 1.0},
        {69 / 255.0, 125 / 255.0, 140 / 255.0, 1.0},
    };
    static Color gradient_morning_blue[4] = {
        {221 / 255.0, 241 / 255.0, 254 / 255.0, 1.0},
        {135 / 255.0, 206 / 255.0, 250 / 255.0, 1.0},
        {0.7, 0.7, 0.7, 1.0},
        {170 / 255.0, 200 / 255.0, 252 / 255.0, 1.0},
    };
    static Color gradient_sunbeam[4] = {
        {20 / 255.0, 13 / 255.0, 20 / 255.0, 1.0},
        {30 / 255.0, 72 / 255.0, 114 / 255.0, 1.0},
        {0.7, 0.7, 0.7, 1.0},
        {0.1, 0.0, 0.1, 1.0},
    };
    static Color gradient_lime_green[4] = {
        {209 / 255.0, 255 / 255.0, 82 / 255.0, 1.0},
        {146 / 255.0, 232 / 255.0, 66 / 255.0, 1.0},
        {82 / 255.0, 101 / 255.0, 35 / 255.0, 1.0},
        {63 / 255.0, 95 / 255.0, 30 / 255.0, 1.0},
    };
    static Color gradient_pikachu_yellow[4] = {
        {63 / 255.0, 63 / 255.0, 1 / 255.0, 1.0},
        {174 / 255.0, 174 / 255.0, 1 / 255.0, 1.0},
        {191 / 255.0, 194 / 255.0, 1 / 255.0, 1.0},
        {254 / 255.0, 221 / 255.0, 3 / 255.0, 1.0},
    };
    static Color gradient_gamecube_purple[4] = {
        {40 / 255.0, 20 / 255.0, 91 / 255.0, 1.0},
        {160 / 255.0, 140 / 255.0, 211 / 255.0, 1.0},
        {107 / 255.0, 92 / 255.0, 177 / 255.0, 1.0},
        {84 / 255.0, 71 / 255.0, 132 / 255.0, 1.0},
    };
    static Color gradient_famicom_red[4] = {
        {255 / 255.0, 191 / 255.0, 171 / 255.0, 1.0},
        {119 / 255.0, 49 / 255.0, 28 / 255.0, 1.0},
        {148 / 255.0, 10 / 255.0, 36 / 255.0, 1.0},
        {206 / 255.0, 126 / 255.0, 110 / 255.0, 1.0},
    };
    static Color gradient_flaming_hot[4] = {
        {231 / 255.0, 53 / 255.0, 53 / 255.0, 1.0},
        {242 / 255.0, 138 / 255.0, 97 / 255.0, 1.0},
        {236 / 255.0, 97 / 255.0, 76 / 255.0, 1.0},
        {255 / 255.0, 125 / 255.0, 3 / 255.0, 1.0},
    };
    static Color gradient_ice_cold[4] = {
        {66 / 255.0, 183 / 255.0, 229 / 255.0, 1.0},
        {29 / 255.0, 164 / 255.0, 255 / 255.0, 1.0},
        {176 / 255.0, 255 / 255.0, 247 / 255.0, 1.0},
        {174 / 255.0, 240 / 255.0, 255 / 255.0, 1.0},
    };
    static Color gradient_midgar[4] = {
        {255 / 255.0, 0 / 255.0, 0 / 255.0, 1.0},
        {0 / 255.0, 0 / 255.0, 255 / 255.0, 1.0},
        {0 / 255.0, 255 / 255.0, 0 / 255.0, 1.0},
        {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
    };

    static Color gradient_volcanic_red[4] = {
        {1.0, 0.0, 0.1, 1.0},
        {1.0, 0.1, 0.0, 1.0},
        {0.1, 0.0, 0.1, 1.0},
        {0.1, 0.0, 0.1, 1.0},
    };
    static Color gradient_dark[4] = {
        {0.05, 0.05, 0.05, 1.0},
        {0.05, 0.05, 0.05, 1.0},
        {0.05, 0.05, 0.05, 1.0},
        {0.05, 0.05, 0.05, 1.0},
    };
    static Color gradient_light[4] = {
        {0.50, 0.50, 0.50, 1.0},
        {0.50, 0.50, 0.50, 1.0},
        {0.50, 0.50, 0.50, 1.0},
        {0.50, 0.50, 0.50, 1.0},
    };
    static Color gradient_gray_dark[4] = {
        {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
        {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
        {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
        {16 / 255.0, 16 / 255.0, 16 / 255.0, 1.0},
    };
    static Color gradient_gray_light[4] = {
        {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
        {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
        {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
        {32 / 255.0, 32 / 255.0, 32 / 255.0, 1.0},
    };

    Color *bgs[] = {
        gradient_golden,          //
        gradient_legacy_red,      //
        gradient_electric_blue,   //
        gradient_dark_purple,     //
        gradient_midnight_blue,   //
        gradient_apple_green,     //
        gradient_undersea,        //
        gradient_morning_blue,    //
        gradient_sunbeam,         //
        gradient_lime_green,      //
        gradient_pikachu_yellow,  //
        gradient_gamecube_purple, //
        gradient_famicom_red,     //
        gradient_flaming_hot,     //
        gradient_ice_cold,        //
        gradient_midgar,          //
        gradient_volcanic_red,    //
        gradient_dark,            //
        gradient_light,           //
        gradient_gray_dark,       //
        gradient_gray_light,      //
    };

    begin_rect(0, 0);
    rect_size(width, height);

    Color *bg = bgs[theme];
    rect_gradient4(bg[2], bg[3], bg[1], bg[0]);
    end_rect();

    // Simple blue gradient background
    // NVGpaint bg_paint = nvgLinearGradient(vg, 0, 0, 0, height, nvgRGBA(0, 20, 50, 255), nvgRGBA(0, 10, 30, 255));
    // nvgBeginPath(vg);
    // nvgRect(vg, 0, 0, width, height);
    // nvgFillPaint(vg, bg_paint);
    // nvgFill(vg);
    //
    // // Add animated wave effect
    // float time = glfwGetTime();
    // for (int i = 0; i < 5; i++) {
    //     float alpha = 0.12f - i * 0.02f;
    //     float offset = i * 80.0f;
    //     float amplitude = 25.0f - i * 4.0f;
    //     float frequency = 0.006f + i * 0.001f;
    //     float speed = 0.7f + i * 0.1f;
    //
    //     nvgBeginPath(vg);
    //     nvgMoveTo(vg, 0, height * 0.7f + sinf(time * speed) * amplitude + offset);
    //
    //     for (int x = 0; x <= width; x += 15) {
    //         float y = height * 0.7f + sinf(time * speed + x * frequency) * amplitude + offset;
    //         nvgLineTo(vg, x, y);
    //     }
    //
    //     nvgLineTo(vg, width, height);
    //     nvgLineTo(vg, 0, height);
    //     nvgClosePath(vg);
    //
    //     NVGcolor wave_color = nvgRGBAf(0.2f, 0.6f, 1.0f, alpha);
    //     nvgFillColor(vg, wave_color);
    //     nvgFill(vg);
    // }
}

void draw_selected_item_title(const HorizontalList *hr_list) {
    Color text_color = {1.0, 1.0, 1.0, 0.9};

    use_font("sans");
    draw_text(24, 30, 40, hr_list->items[hr_list->selected].title, text_color);

    // subtitle
    draw_text(16, 30, 70, hr_list->items[hr_list->selected].path, text_color);
}

void draw_horizontal_menu(const HorizontalList *hr_list, int x, int y) {
    float gap = 150.0f;
    float base_x = x - hr_list->scroll;

    // draw title
    draw_selected_item_title(hr_list);
    float size = 50;

    if (hr_list->depth > 0) {
        float scale_factor = 1.5f;
        size *= scale_factor;
        float x = base_x + (hr_list->selected * gap);

        begin_rect(x - size / 2, y - size / 2);
        rect_size(size, size);
        rect_radius(20, 20, 20, 20);
        rect_color(1, 1, 1, 1);
        end_rect();

        // icon
        use_font("icon");
        float fsize = 30 * scale_factor;
        Color icon_color = {0, 0, 0, 1.0};

        float w, h;
        get_text_bounds(fsize, hr_list->items[hr_list->selected].icon, &w, &h, NULL, NULL);
        draw_text(fsize, x - w / 2, y + h / 2, hr_list->items[hr_list->selected].icon, icon_color);
        return;
    }

    use_font("icon");
    Color iconColor = {0, 0.07, 0.19, 1};
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

        // Draw category icon
        begin_rect(x - size / 2, y - size / 2);
        rect_size(size, size);
        rect_radius_all(14 * scale_factor);
        rect_color(1, 1, 1, opacity);
        end_rect();

        // icon
        float fsize = 20 * scale_factor;

        float w, h, by, sx;
        get_text_bounds(fsize, hr_list->items[i].icon, &w, &h, &sx, &by);

        float ty = y + by / 2;
        float tx = x - w / 2 - sx;

        draw_text(fsize, tx, ty, hr_list->items[i].icon, iconColor);
    }
}

void draw_folder_path(const HorizontalList *hr_list, const char *path, float x, float y) {
    if (hr_list->depth == 0)
        return;

    use_font("sans");
    draw_text(12, x, y, path, (Color){1, 1, 1, 1});
}

void draw_vertical_list(const VerticalList *list, float start_x) {
    if (list->items_count == 0)
        return;

    for (int i = list->entry_start; i < list->entry_end; i++) {
        struct file_entry *node = list->items[i];

        float x = start_x + node->x;
        float y = list->margins_screen_top + node->y;

        Color icon_color = {0.8, 0.8, 1, node->alpha};

        float size = list->icon_size + (node->zoom == 1 ? 10 : 0);

        use_font("icon");
        draw_text(size, x - size / 2, y + size / 2 - 2, node->type == TYPE_DIRECTORY ? "\ue950" : "\ue96d", icon_color);

        Color text_color = {1, 1, 1, node->label_alpha};

        use_font("sans");

        char name[60];
        if (strlen(node->name) > 59) {
            memcpy(name, &node->name, 59);
            name[59] = '\0';
        } else {
            memcpy(name, &node->name, strlen(node->name));
            name[strlen(node->name)] = '\0';
        }

        float w, h;
        float fsize = 12 + (node->zoom == 1 ? 4 : 0);
        get_text_bounds(fsize, name, &w, &h, NULL, NULL);
        draw_text(fsize, x + 50, y + h / 2 - 4, name, text_color);
    }

    // selected item always in place
    float y = list->margins_screen_top + list->icon_spacing_vertical;
    float pulse = 0.5f + 0.3f * sinf(glfwGetTime() * 3.0f);

    begin_rect(start_x + 35, y - 20);
    rect_size(600, 40);
    rect_radius(10, 10, 10, 10);
    rect_color(1, 1, 1, 0.2 * pulse);
    end_rect();
}

void draw_text_preview(const char *text, float width, float height) {
    float x = 40;
    float y = 40;
    width -= 80;
    height -= 80;

    begin_rect(x, y);
    rect_size(width, height);
    rect_radius(20, 20, 20, 20);
    rect_color(0, 0, 0, .8);
    end_rect();

    float padding = 20;

    x += padding;
    y += padding;
    width -= padding * 2;

    use_font("sans");
    draw_wrapped_text(16, x, y + 10, text, (Color){1, 1, 1, 1}, width);
}

char *readable_fs(double bytes, char *buf) {
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (bytes > 1024) {
        bytes /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, bytes, units[i]);
    return buf;
}

void draw_info(const VerticalList *vr_list, float width, float height) {
    struct file_entry *current = vr_list->items[vr_list->selected];
    if (current == NULL)
        return;

    float rect_w = 500;
    float x = width - rect_w;
    float y = height - 200;

    begin_rect(x, y);
    rect_size(rect_w - 20, 180);
    rect_radius(10, 10, 10, 10);
    rect_color(0, 0, 0, .6);
    end_rect();

    float gap = 30;

    y += gap;

    use_font("sans");
    draw_text(16, x + 20, y, "Name:", (Color){1, 1, 1, 1});
    draw_text(16, x + 180, y, current->name, (Color){1, 1, 1, 1});

    y += gap;

    draw_text(16, x + 20, y, "Path:", (Color){1, 1, 1, 1});
    draw_text(16, x + 180, y, current->path, (Color){1, 1, 1, 1});

    y += gap;

    struct tm access_lt;
    localtime_r(&current->access_time, &access_lt);
    char access_timbuf[80];
    strftime(access_timbuf, sizeof(access_timbuf), "%c", &access_lt);
    draw_text(16, x + 20, y, "Access Time:", (Color){1, 1, 1, 1});
    draw_text(16, x + 180, y, access_timbuf, (Color){1, 1, 1, 1});

    y += gap;

    struct tm modified_lt;
    localtime_r(&current->modified_time, &modified_lt);
    char modified_timbuf[80];
    strftime(modified_timbuf, sizeof(access_timbuf), "%c", &access_lt);
    draw_text(16, x + 20, y, "Modified Time:", (Color){1, 1, 1, 1});
    draw_text(16, x + 180, y, modified_timbuf, (Color){1, 1, 1, 1});

    y += gap;

    char buf[20];
    draw_text(16, x + 20, y, "Size:", (Color){1, 1, 1, 1});
    draw_text(16, x + 180, y, readable_fs(current->size, buf), (Color){1, 1, 1, 1});
}
