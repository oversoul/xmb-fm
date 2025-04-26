#include "draw.h"
#include "ui.h"
#include "hr_list.h"
#include "vr_list.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        float fsize = 20 * scale_factor;
        Color icon_color = {0, 0, 0, 1};

        float w, h, by, sx;
        get_text_bounds(fsize, hr_list->items[hr_list->selected].icon, &w, &h, &sx, &by);

        float ty = y + by / 2;
        float tx = x - w / 2 - sx;
        draw_text(fsize, tx, ty, hr_list->items[hr_list->selected].icon, icon_color);

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

        Color icon_color = {1, 1, 1, node->alpha};

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

        float w, h, by, sx;
        float fsize = 12 + (node->zoom == 1 ? 4 : 0);
        get_text_bounds(fsize, name, &w, &h, &sx, &by);

        float ty = y + by / 2;
        float tx = x - sx;

        draw_text(fsize, tx + 50, ty, name, text_color);
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

void draw_text_preview(DrawState *state) {
    if (!state->show_preview)
        return;

    begin_rect(0, 0);
    rect_size(state->width, state->height);
    rect_color(0, 0, 0, .8);
    end_rect();

    float x = state->width / 10.0;
    float y = state->height / 10.0;

    begin_rect(x, y);
    rect_size(state->width - 2 * x, state->height - 2 * y);
    rect_radius(20, 20, 20, 20);
    rect_color(1, 1, 1, 1);
    end_rect();

    float padding = 20;
    float content_width = state->width - 2 * x;

    x += padding;
    y += padding;
    content_width -= padding * 2;

    use_font("sans");
    draw_wrapped_text(16, x, y + 10, state->buffer, (Color){0, 0, 0, 1}, content_width);
}

static char *readable_fs(double bytes, char *buf) {
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (bytes > 1024) {
        bytes /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, bytes, units[i]);
    return buf;
}

static float draw_section(float x, float y, const char *key, const char *value, Color color, float max_width) {
    draw_text(16, x, y, key, color);
    draw_wrapped_text(16, x + 160, y, value, color, max_width);
    return 16;
}

void draw_info(const VerticalList *vr_list, float width, float height) {
    struct file_entry *current = vr_list->items[vr_list->selected];
    if (current == NULL)
        return;

    use_font("sans");
    Color color = {1, 1, 1, 1};

    char buf[20];
    struct tm access_lt;
    localtime_r(&current->access_time, &access_lt);
    char access_timbuf[80];
    strftime(access_timbuf, sizeof(access_timbuf), "%c", &access_lt);

    struct tm modified_lt;
    localtime_r(&current->modified_time, &modified_lt);
    char modified_timbuf[80];
    strftime(modified_timbuf, sizeof(access_timbuf), "%c", &access_lt);

    float x = 100;
    float y = 100;
    float gap = 20;
    float wrap_width = width - 40;
    float total_height = 5 * 16 + gap * 4;
    y = height / 2 - total_height / 2;

    y += draw_section(x, y, "         Name", current->name, color, wrap_width) + gap;
    y += draw_section(x, y, "         Path", current->path, color, wrap_width) + gap;
    y += draw_section(x, y, "  Access Time", access_timbuf, color, wrap_width) + gap;
    y += draw_section(x, y, "Modified Time", modified_timbuf, color, wrap_width) + gap;
    y += draw_section(x, y, "         Size", readable_fs(current->size, buf), color, wrap_width);
}

void draw_option_list_depth(float x, float y, Options *list) {
    Color color = {1, 1, 1, 1};
    Color muted_color = {1, 1, 1, .5};
    float rect_w = OPTION_LIST_WIDTH;

    for (size_t i = 0; i < list->items_count; ++i) {
        use_font("sans");
        draw_text(16, x + 20, y + 40 * i, list->items[i].title, i == list->selected ? color : muted_color);
        if (list->items[i].submenu) {
            use_font("icon");
            draw_text(16, x + rect_w - 40, y + 40 * i, "\ue942", i == list->selected ? color : muted_color);
        }
    }
}

void draw_option_list(OptionList *op_list, DrawState *state) {
    float rect_w = OPTION_LIST_WIDTH;
    float x = state->width + op_list->x;

    begin_rect(x, 0);
    rect_size(x + state->width, state->height);
    rect_gradient_sides((Color){0, 0, 0, .8}, (Color){0, 0, 0, .3});
    end_rect();

    Options *menus[op_list->depth == 0 ? 1 : op_list->depth]; // Assuming max depth of 10
    int depth = 0;

    // Start from current and go to root
    Options *current = op_list->current;
    while (current) {
        menus[depth++] = current;
        current = current->parent;
    }

    for (int i = depth - 1; i >= 0; i--) {
        float total_height = 40. * menus[i]->items_count;
        float start_y = state->height / 2.0 - total_height / 2.0;

        draw_option_list_depth(x + (depth - 1 - i) * rect_w, start_y, menus[i]);
    }
}

void draw_search_field(DrawState *state) {
    if (!state->show_search)
        return;

    begin_rect(0, 0);
    rect_size(state->width, state->height);
    rect_color(0, 0, 0, .3);
    end_rect();

    float w = 400;
    float h = 200;

    float x = state->width / 2.0 - w / 2.0;
    float y = state->height / 2.0 - h / 2.0;
    begin_rect(x, y);
    rect_size(w, h);
    rect_radius(20, 20, 20, 20);
    rect_color(1, 1, 1, 1);
    end_rect();

    float padding = 20;

    x += padding;
    y += padding;

    use_font("sans");
    const char *text_field = "Search";
    float tw, th;
    get_text_bounds(16, text_field, &tw, &th, NULL, NULL);
    draw_text(16, state->width / 2.0 - tw / 2.0, y + padding, text_field, (Color){0, 0, 0, 1});

    draw_text(22, x, y + h / 2, state->search_buffer, (Color){0, 0, 0, 1});
}
