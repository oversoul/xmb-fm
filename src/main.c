#include "animation.h"
#include "fm.h"
#include <GL/glew.h>
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

// #define NANOVG_GL3_IMPLEMENTATION
// #include "nanovg.h"
// #include "nanovg_gl.h"

#include "hr_list.h"
#include "draw.h"
#include "vr_list.h"

// Global state
typedef struct {
    int width;
    int height;
    int depth;
    bool show_info;
    bool show_preview;
    char buffer[512];
} State;

int theme = 0;
State state = {0};
HrItem horizontalItems[10];

FileManager *fm;
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

void vr_list_update() {
    vr_list.items = fm->current_dir->children;
    vr_list.items_count = fm->current_dir->child_count;
    update_vertical_list(&vr_list, glfwGetTime());
}

// Input handling
void handle_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;

    if (state.show_preview) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_P) {
            memset(state.buffer, 0, 512);
            state.show_preview = false;
        }
        return;
    }

    switch (key) {
    case GLFW_KEY_ESCAPE: {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } break;
    case GLFW_KEY_EQUAL: {
        if (theme < 20)
            theme++;
    } break;
    case GLFW_KEY_MINUS: {
        if (theme > 0)
            theme--;
    } break;
    case GLFW_KEY_LEFT: {
        if (state.depth > 0)
            return;
        if (hr_list.selected > 0) {
            hr_list.selected--;
            update_horizontal_list(&hr_list, glfwGetTime());
            while (fm->history_pos != 0)
                go_back(fm);
            state.depth = 0;

            change_directory(fm, horizontalItems[hr_list.selected].path);

            vr_list.selected = 0;
            vr_list_update();
        }
    } break;
    case GLFW_KEY_RIGHT: {
        if (state.depth > 0)
            return;

        if (hr_list.selected < hr_list.items_count - 1) {
            hr_list.selected++;
            update_horizontal_list(&hr_list, glfwGetTime());
            while (fm->history_pos != 0)
                go_back(fm);
            state.depth = 0;

            change_directory(fm, horizontalItems[hr_list.selected].path);

            vr_list.selected = 0;
            vr_list_update();
        }
    } break;
    case GLFW_KEY_UP: {
        if (vr_list.selected > 0) {
            vr_list.selected--;

            vr_list_update();
        }
    } break;
    case GLFW_KEY_DOWN: {
        if (vr_list.selected < vr_list.entry_end - 1) {
            vr_list.selected++;

            vr_list_update();
        }
    } break;

    case GLFW_KEY_P: {
        struct file_entry *current = fm->current_dir->children[vr_list.selected];
        if (current->type == TYPE_FILE && is_text_file(current->path, state.buffer, 512)) {
            state.show_preview = true;
        }
    } break;
    case GLFW_KEY_I: {
        state.show_info = !state.show_info;
    } break;
    case GLFW_KEY_BACKSPACE: {
        if (state.depth == 0)
            return;
        state.depth--;
        if (state.depth == 0)
            hr_list.depth = 0;

        const char *old = fm->current_dir->path;

        go_back(fm);

        vr_list.selected = find_index_of(fm, old, 0);

        vr_list_update();
        update_horizontal_list(&hr_list, glfwGetTime());
    } break;
    case GLFW_KEY_ENTER: {
        struct file_entry *current = fm->current_dir->children[vr_list.selected];
        if (current->type == TYPE_DIRECTORY) {
            state.depth++;

            hr_list.depth = 1;

            change_directory(fm, current->path);
            vr_list.selected = 0;
            vr_list_update();
            update_horizontal_list(&hr_list, glfwGetTime());
        } else if (current->type == TYPE_FILE) {
            char command[1060];
            sprintf(command, "xdg-open \"%s\" &", current->path);
            system(command);
        }
    } break;
    }
}

// Initialization of menu data
void initialize_menu_data() {
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    // Users category
    strcpy(horizontalItems[0].title, "Home");
    strcpy(horizontalItems[0].path, homedir);
    strcpy(horizontalItems[0].icon, "");

    // Settings category
    strcpy(horizontalItems[1].title, "Desktop");
    sprintf(horizontalItems[1].path, "%s/%s", homedir, "Desktop");
    strcpy(horizontalItems[1].icon, "");

    // Add more categories like in the Vue code
    strcpy(horizontalItems[2].title, "Documents");
    sprintf(horizontalItems[2].path, "%s/%s", homedir, "Documents");
    strcpy(horizontalItems[2].icon, "");

    // Songs category
    strcpy(horizontalItems[3].title, "Downloads");
    sprintf(horizontalItems[3].path, "%s/%s", homedir, "Downloads");
    strcpy(horizontalItems[3].icon, "");

    // Movies category
    strcpy(horizontalItems[4].title, "Pictures");
    sprintf(horizontalItems[4].path, "%s/%s", homedir, "Pictures");
    strcpy(horizontalItems[4].icon, "");

    // Games category
    strcpy(horizontalItems[5].title, "Public");
    sprintf(horizontalItems[5].path, "%s/%s", homedir, "Public");
    strcpy(horizontalItems[5].icon, "");

    // Network category
    strcpy(horizontalItems[6].title, "Videos");
    sprintf(horizontalItems[6].path, "%s/%s", homedir, "Videos");
    strcpy(horizontalItems[6].icon, "");

    // Friends category
    strcpy(horizontalItems[7].title, "File System");
    strcpy(horizontalItems[7].path, "/");
    strcpy(horizontalItems[7].icon, "");

    fm = create_file_manager(horizontalItems[hr_list.selected].path);

    // Initialize animation state
    hr_list.items = horizontalItems;
    hr_list.items_count = 8;

    // vr
    vr_list.above_subitem_offset = 0.0f;
    vr_list.above_item_offset = -1.5f;
    vr_list.active_item_factor = 1.0f;
    vr_list.under_item_offset = 1.0f;

    vr_list.icon_size = 28.0;
    vr_list.margins_screen_top = 200;
    vr_list.icon_spacing_vertical = 50.0;

    vr_list.get_screen_size = get_window_size;
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
    GLFWwindow *window = glfwCreateWindow(960, 720, "NanoVG", NULL, NULL);
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

    // Initialize NanoVG
    // NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    // if (vg == NULL) {
    //     fprintf(stderr, "Failed to initialize NanoVG\n");
    //     glfwTerminate();
    //     return -1;
    // }

    // Load font
    // int fontNormal = nvgCreateFont(vg, "sans", "./fonts/SpaceMonoNerdFont.ttf");
    // if (fontNormal == -1) {
    //     printf("Could not add font.\n");
    //     return -1;
    // }
    //
    // int fontIcon = nvgCreateFont(vg, "icon", "./fonts/feather.ttf");
    // if (fontIcon == -1) {
    //     printf("Could not add font.\n");
    //     return -1;
    // }

    // glfwSetTime(0);
    glfwSwapInterval(1);

    ui_create();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Register a font
    if (register_font("sans", "./fonts/SpaceMonoNerdFont.ttf") < 0) {
        fprintf(stderr, "Failed to register font\n");
        ui_delete();
        return -1;
    }

    int fontIcon = register_font("icon", "./fonts/feather.ttf");
    if (fontIcon == -1) {
        printf("Could not add font.\n");
        ui_delete();
        return -1;
    }

    // Initialize menu data
    srand(time(NULL));
    initialize_menu_data();

    vr_list_update();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();

        gfx_animation_update(current_time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        start_frame(width, height);

        draw_background(state.width, state.height, theme);
        draw_folder_path(&hr_list, fm->current_dir->path);
        draw_vertical_list(&vr_list);
        draw_horizontal_menu(&hr_list, 180, 150);

        if (state.show_preview) {
            draw_text_preview(state.buffer, state.width, state.height);
        }

        if (state.show_info) {
            draw_info(&vr_list, state.width, state.height);
        }

        end_frame();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_file_manager(fm);
    gfx_animation_clean();

    ui_delete();
    // Cleanup
    // nvgDeleteGL3(vg);

    glfwTerminate();

    return 0;
}
