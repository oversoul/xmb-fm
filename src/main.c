#include "animation.h"
#include "fm.h"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <GLFW/glfw3.h>
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

#include "hr_list.h"
#include "vr_list.h"

// Global state
typedef struct {
    int width, height;
    int depth;
} State;

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

// Input handling
void handle_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    case GLFW_KEY_LEFT:
        if (state.depth > 0) {
            return;
        }
        if (hr_list.selected > 0) {
            hr_list.selected--;
            while (fm->history_pos != 0)
                go_back(fm);
            state.depth = 0;

            change_directory(fm, horizontalItems[hr_list.selected].path);

            vr_list.selected = 0;
        }
        break;
    case GLFW_KEY_RIGHT:
        if (state.depth > 0) {
            return;
        }
        if (hr_list.selected < hr_list.items_count - 1) {
            hr_list.selected++;
            while (fm->history_pos != 0)
                go_back(fm);
            state.depth = 0;

            change_directory(fm, horizontalItems[hr_list.selected].path);

            vr_list.selected = 0;
        }
        break;
    case GLFW_KEY_UP:
        if (vr_list.selected > 0) {
            vr_list.selected--;
        }
        break;
    case GLFW_KEY_DOWN:
        if (vr_list.selected < vr_list.items_count - 1) {
            vr_list.selected++;
        }
        break;
    case GLFW_KEY_BACKSPACE:
        if (state.depth == 0)
            return;
        state.depth--;
        if (state.depth == 0) {
            hr_list.depth = 0;
        }
        go_back(fm);
        vr_list.selected = 0;
        break;
    case GLFW_KEY_ENTER:
        struct file_entry *current = fm->current_dir->children[vr_list.selected];
        if (current->type == TYPE_DIRECTORY) {
            printf("NAVIGATE: %s\n", current->name);
            state.depth++;

            hr_list.depth = 1;
            change_directory(fm, current->path);
            vr_list.selected = 0;
        } else if (current->type == TYPE_FILE) {
            // open
            printf("OPEN: %s\n", current->path);
            char command[1060];
            sprintf(command, "xdg-open \"%s\" &", current->path);
            system(command);
        }
        break;
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
    strcpy(horizontalItems[0].icon, "");

    // Settings category
    strcpy(horizontalItems[1].title, "Desktop");
    sprintf(horizontalItems[1].path, "%s/%s", homedir, "Desktop");
    strcpy(horizontalItems[1].icon, "");

    // Add more categories like in the Vue code
    strcpy(horizontalItems[2].title, "Documents");
    sprintf(horizontalItems[2].path, "%s/%s", homedir, "Documents");
    strcpy(horizontalItems[2].icon, "󱔗");

    // Songs category
    strcpy(horizontalItems[3].title, "Downloads");
    sprintf(horizontalItems[3].path, "%s/%s", homedir, "Downloads");
    strcpy(horizontalItems[3].icon, "");

    // Movies category
    strcpy(horizontalItems[4].title, "Pictures");
    sprintf(horizontalItems[4].path, "%s/%s", homedir, "Pictures");
    strcpy(horizontalItems[4].icon, "");

    // Games category
    strcpy(horizontalItems[5].title, "Public");
    sprintf(horizontalItems[5].path, "%s/%s", homedir, "Public");
    strcpy(horizontalItems[5].icon, "");

    // Network category
    strcpy(horizontalItems[6].title, "Videos");
    sprintf(horizontalItems[6].path, "%s/%s", homedir, "Videos");
    strcpy(horizontalItems[6].icon, "󰕧");

    // Friends category
    strcpy(horizontalItems[7].title, "File System");
    strcpy(horizontalItems[7].path, "/");
    strcpy(horizontalItems[7].icon, "");

    fm = create_file_manager(horizontalItems[hr_list.selected].path);
    // sort_entries(fm);

    // Initialize animation state
    hr_list.items = horizontalItems;
    hr_list.items_count = 8;

    // vr
    vr_list.above_subitem_offset = 0.0f;
    vr_list.above_item_offset = -2.0f;
    vr_list.active_item_factor = 1.0f;
    vr_list.under_item_offset = 1.0f;

    vr_list.icon_size = 28.0;
    vr_list.margins_screen_top = 200;
    vr_list.icon_spacing_vertical = 40.0;

    vr_list.get_screen_size = get_window_size;
}

// Drawing functions
void draw_background(NVGcontext *vg) {
    // Simple blue gradient background
    NVGpaint bg_paint = nvgLinearGradient(vg, 0, 0, 0, state.height, nvgRGBA(0, 20, 50, 255), nvgRGBA(0, 10, 30, 255));
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, state.width, state.height);
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
        nvgMoveTo(vg, 0, state.height * 0.7f + sinf(time * speed) * amplitude + offset);

        for (int x = 0; x <= state.width; x += 15) {
            float y = state.height * 0.7f + sinf(time * speed + x * frequency) * amplitude + offset;
            nvgLineTo(vg, x, y);
        }

        nvgLineTo(vg, state.width, state.height);
        nvgLineTo(vg, 0, state.height);
        nvgClosePath(vg);

        NVGcolor wave_color = nvgRGBAf(0.2f, 0.6f, 1.0f, alpha);
        nvgFillColor(vg, wave_color);
        nvgFill(vg);
    }
}

// Main rendering function
void render(GLFWwindow *window, NVGcontext *vg) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    glViewport(0, 0, width, height);
    nvgBeginFrame(vg, winWidth, winHeight, (float)winWidth / (float)winHeight);

    // Draw background
    draw_background(vg);

    if (hr_list.depth > 0) {
        float x = 200;
        nvgFontSize(vg, 12);
        nvgFontFace(vg, "sans");
        nvgFillColor(vg, nvgRGB(255, 255, 255));
        nvgText(vg, x, 160, fm->current_dir->path, NULL);

        // for (size_t i = 0; i < fm->history_pos; ++i) {
        //     float bounds[4];
        //     nvgTextBounds(vg, 0, 0, fm->history[i]->name, NULL, bounds);
        //     printf("BOUNDS: %f\n", bounds[2]);
        //     nvgText(vg, x, 160, fm->history[i]->name, NULL);
        //     x += bounds[2] + 10;
        // }
    }

    draw_ui(&vr_list, vg);

    draw_horizontal_menu(vg, &hr_list, width * 0.2f, 150);

    // draw preview image

    nvgEndFrame(vg);
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
    NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        fprintf(stderr, "Failed to initialize NanoVG\n");
        glfwTerminate();
        return -1;
    }

    // Load font
    int fontNormal = nvgCreateFont(vg, "sans", "./fonts/SpaceMonoNerdFont.ttf");
    if (fontNormal == -1) {
        printf("Could not add font.\n");
        return -1;
    }

    int fontIcon = nvgCreateFont(vg, "icon", "./fonts/feather.ttf");
    if (fontIcon == -1) {
        printf("Could not add font.\n");
        return -1;
    }

    // glfwSetTime(0);
    glfwSwapInterval(1);

    // Initialize menu data
    srand(time(NULL));
    initialize_menu_data();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();

        gfx_animation_update(current_time);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        vr_list.items = fm->current_dir->children;
        vr_list.items_count = fm->current_dir->child_count;

        selection_pointer_changed(&vr_list);
        update_horizontal_list(&hr_list);

        // Render menu
        render(window, vg);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_file_manager(fm);
    gfx_animation_clean();

    // Cleanup
    nvgDeleteGL3(vg);

    glfwTerminate();

    return 0;
}
