#include "animation.h"
#include "fm.h"
#include <GL/glew.h>
#include "option_list.h"
#include "ribbon.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <GLFW/glfw3.h>

#include "hr_list.h"
#include "draw.h"
#include "vr_list.h"

// Global state
typedef struct {
    int theme;
    int width;
    int height;
    bool show_info;
    bool show_preview;
    char buffer[512];
} State;

State state = {0};

FileManager *fm;
OptionList op_list;
VerticalList vr_list;
HorizontalList hr_list;

void resize_callback(GLFWwindow *window, int w, int h) {
    glViewport(0, 0, w, h);
    state.width = w;
    state.height = h;
}

void get_window_size(unsigned *width, unsigned *height) {
    if (width) {
        *width = state.width;
    }
    if (height) {
        *height = state.height;
    }
}

void hr_list_update(float current_time) {
    animation_remove_by_tag(HorizontalListTag);

    update_horizontal_list(&hr_list, current_time);
}

void vr_list_update(float current_time) {
    animation_remove_by_tag(VerticalListTag);

    vr_list.items = fm->current_dir->children;
    vr_list.items_count = fm->current_dir->child_count;
    update_vertical_list(&vr_list, current_time);
}

bool handle_global_key(int key, float current_time) {
    switch (key) {
    case GLFW_KEY_EQUAL:
        if (state.theme < 20)
            state.theme++;
        return true;
    case GLFW_KEY_MINUS:
        if (state.theme > 0)
            state.theme--;
        return true;
    }
    return false;
}

bool handle_hr_list_key(int key, float current_time) {
    if (hr_list.depth > 0)
        return false;

    bool updated = false;
    if (key == GLFW_KEY_LEFT && hr_list.selected > 0) {
        hr_list.selected--;
        updated = true;
    } else if (key == GLFW_KEY_RIGHT && hr_list.selected < hr_list.items_count - 1) {
        hr_list.selected++;
        updated = true;
    }

    if (updated) {
        hr_list_update(current_time);
        switch_directory(fm, hr_list.items[hr_list.selected].path);
        vr_list.selected = 0;
        vr_list_update(current_time);
        return true;
    }

    return false;
}

bool handle_vr_list_key(int key, float current_time) {
    bool updated = false;

    if (key == GLFW_KEY_UP && vr_list.selected > 0) {
        vr_list.selected--;
        updated = true;
    }

    if (key == GLFW_KEY_DOWN && vr_list.selected < vr_list.entry_end - 1) {
        vr_list.selected++;
        updated = true;
    }

    if (key == GLFW_KEY_PAGE_UP && vr_list.selected > 0) {
        vr_list.selected = (vr_list.selected > 10) ? vr_list.selected - 10 : 0;
        updated = true;
    }

    if (key == GLFW_KEY_PAGE_DOWN && vr_list.selected < vr_list.entry_end - 1) {
        vr_list.selected = (vr_list.selected < vr_list.entry_end - 10) ? vr_list.selected + 10 : vr_list.entry_end - 1;
        updated = true;
    }

    if (key == GLFW_KEY_HOME) {
        vr_list.selected = 0;
        updated = true;
    }

    if (key == GLFW_KEY_END) {
        vr_list.selected = vr_list.items_count - 1;
        updated = true;
    }

    if (updated) {
        vr_list_update(current_time);
        return true;
    }

    return false;
}

bool handle_file_entry_key(int key, float current_time) {
    struct file_entry *current = fm->current_dir->children[vr_list.selected];

    switch (key) {
    case GLFW_KEY_BACKSPACE:
        if (hr_list.depth == 0)
            return false;
        hr_list.depth--;
        vr_list.selected = navigate_back(fm);
        vr_list_update(current_time);
        hr_list_update(current_time);
        return true;
    case GLFW_KEY_ENTER:
        if (current->type == TYPE_DIRECTORY) {
            hr_list.depth++;
            change_directory(fm, current->path);
            vr_list.selected = 0;
            vr_list_update(current_time);
            hr_list_update(current_time);
        } else if (current->type == TYPE_FILE) {
            open_file(current->path);
        }
        return true;
    case GLFW_KEY_P:
        if (current->type == TYPE_FILE && get_mime_type(current->path, "text/")) {
            read_file_content(current->path, state.buffer, 512);
            state.show_preview = true;
        }
        return true;
    }
    return false;
}

void handle_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;

    // Global ESC to close info or preview
    if (state.show_info && key == GLFW_KEY_ESCAPE) {
        state.show_info = false;
        return;
    }

    if (state.show_preview && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_P)) {
        memset(state.buffer, 0, 512);
        state.show_preview = false;
        return;
    }

    float current_time = glfwGetTime();

    // Option list mode
    if (handle_option_list_key(&op_list, key, current_time))
        return;

    // Main view mode
    if (handle_global_key(key, current_time))
        return;
    if (handle_hr_list_key(key, current_time))
        return;
    if (handle_vr_list_key(key, current_time))
        return;
    if (handle_file_entry_key(key, current_time))
        return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

void op_list_option_selected(Option *option) {
    if (strcmp(option->title, "Information") == 0) {
        state.show_info = true;
    }
}

// Initialization of menu data
void initialize_menu_data() {
    init_horizontal_list(&hr_list);

    fm = create_file_manager(hr_list.items[hr_list.selected].path);

    // vr
    init_vertical_list(&vr_list);
    vr_list.get_screen_size = get_window_size;

    // op
    op_list.on_item_selected = op_list_option_selected;

    op_list.items_count = 5;
    op_list.items = malloc(sizeof(Option) * op_list.items_count);
    op_list.items[0] = (Option){"Cut"};
    op_list.items[1] = (Option){"Copy"};
    op_list.items[2] = (Option){"Rename"};
    op_list.items[3] = (Option){"Delete"};
    op_list.items[4] = (Option){"Information"};
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow *window = glfwCreateWindow(960, 720, "File Manager", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowAttrib(window, GLFW_FLOATING, GL_TRUE);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, handle_key);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Could not init glew.\n");
        return 1;
    }

    glfwSwapInterval(1);

    ui_create();

    init_ribbon();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Register a font
    if (register_font("sans", "./fonts/SpaceMonoNerdFont.ttf") < 0) {
        fprintf(stderr, "Failed to register font\n");
        ui_delete();
        return -1;
    }

    if (register_font("icon", "./fonts/feather.ttf") < 0) {
        fprintf(stderr, "Failed to register font icon\n");
        ui_delete();
        return -1;
    }

    state.theme = 2; // electric_blue
    srand(time(NULL));
    initialize_menu_data();

    vr_list_update(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();

        animation_update(current_time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        start_frame(width, height);
        draw_background(state.width, state.height, state.theme);

        if (!state.show_info) {
            draw_folder_path(&hr_list, fm->current_dir->path, 200, 160);
            draw_vertical_list(&vr_list, 180);
            draw_horizontal_menu(&hr_list, 180, 150);

            if (state.show_preview) {
                draw_text_preview(state.buffer, state.width, state.height);
            }

            draw_option_list(&op_list, state.width, state.height);

        } else {
            draw_info(&vr_list, state.width, state.height);
        }

        end_frame();

        draw_ribbon(state.width, state.height, current_time);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(op_list.items);
    free_file_manager(fm);
    animation_clean();

    ui_delete();

    glfwTerminate();

    return 0;
}
