#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include "animation.h"
#include "fm.h"
#include "input.h"
#include "option_list.h"
#include "ribbon.h"
#include "signal.h"
#include "ui.h"
#include "draw.h"
#include "hr_list.h"
#include "vr_list.h"

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a < b ? b : a)

// Global state

DrawState state = {0};

FileManager *fm;
Input search_input;
Input rename_input;
OptionList op_list;
VerticalList vr_list;
HorizontalList hr_list;

Option options_items[] = {
    {.title = "Option 1"},
    {.title = "Option 2"},
    {.title = "Option 3"},
};

Options options = {
    .selected = 0,
    .items = options_items,
    .items_count = sizeof(options_items) / sizeof(Option),
    .parent = NULL,
};

Option root_items[] = {
    {.title = "Cut"},         //
    {.title = "Copy"},        //
    {.title = "Rename"},      //
    {.title = "Delete"},      //
    {.title = "Information"}, //
    {.title = "More"},        //
};

Options root = {
    .items = root_items,
    .items_count = sizeof(root_items) / sizeof(Option),
    .selected = 0,
    .parent = NULL,
};

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

bool handle_global_key(int key) {
    if (key == GLFW_KEY_EQUAL) {
        state.theme = min(state.theme + 1, 20);
        return true;
    }

    if (key == GLFW_KEY_MINUS) {
        state.theme = max(state.theme - 1, 0);
        return true;
    }

    return false;
}

bool handle_hr_list_key(int key) {
    if (hr_list.depth > 0)
        return false;

    if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) {
        int selected = 0;
        if (key == GLFW_KEY_RIGHT)
            selected = min(hr_list.selected + 1, hr_list.items_count - 1);
        else
            selected = max(hr_list.selected - 1, 0);

        if (selected == hr_list.selected)
            return false;

        hr_list.selected = selected;

        // Emit selection change event
        SelectionData sel_data = {.index = selected};
        emit_signal(EVENT_HORIZONTAL_SELECTION_CHANGED, &sel_data);

        return true;
    }

    return false;
}

bool handle_option_list_key(OptionList *list, int key) {
    if (list->depth > 0) {
        switch (key) {
        case GLFW_KEY_I:
        case GLFW_KEY_ESCAPE:
            option_list_event_handler(OPTION_EVENT_CLOSE_MENU, list, NULL);
            return true;

        case GLFW_KEY_UP: {
            int direction = -1;
            option_list_event_handler(OPTION_EVENT_MOVE_SELECTION, list, &direction);
            return true;
        }

        case GLFW_KEY_DOWN: {
            int direction = 1;
            option_list_event_handler(OPTION_EVENT_MOVE_SELECTION, list, &direction);
            return true;
        }

        case GLFW_KEY_ENTER: {
            Option *current = &list->current->items[list->current->selected];

            if (current->submenu && current->submenu->items_count > 0) {
                option_list_event_handler(OPTION_EVENT_OPEN_SUBMENU, list, NULL);
            } else {
                option_list_event_handler(OPTION_EVENT_SELECT_ITEM, list, NULL);
            }
            return true;
        }
        }
        return true;
    }

    if (key == GLFW_KEY_I) {
        option_list_event_handler(OPTION_EVENT_OPEN_MENU, list, NULL);
        return true;
    }

    return false;
}

bool handle_vr_list_key(int key) {
    int selected = vr_list.selected;

    switch (key) {
    case GLFW_KEY_BACKSPACE:
        emit_signal(EVENT_NAVIGATE_BACK, NULL);
        return true;
    case GLFW_KEY_ENTER:
        emit_signal(EVENT_ITEM_ACTIVATED, &vr_list.selected);
        return true;
    case GLFW_KEY_UP:
        selected = max(selected - 1, 0);
        break;
    case GLFW_KEY_DOWN:
        selected = min(selected + 1, vr_list.entry_end - 1);
        break;

    case GLFW_KEY_PAGE_UP:
        selected = max(selected - 10, 0);
        break;

    case GLFW_KEY_PAGE_DOWN:
        selected = min(selected + 10, vr_list.entry_end - 1);
        break;

    case GLFW_KEY_HOME:
        selected = 0;
        break;

    case GLFW_KEY_END:
        selected = vr_list.items_count - 1;
        break;
    }

    if (selected != vr_list.selected) {
        SelectionData sel_data = {.index = selected};
        emit_signal(EVENT_VERTICAL_SELECTION_CHANGED, &sel_data);
        return true;
    }

    return false;
}

bool handle_file_entry_key(int key) {
    if (state.show_preview) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_P) {
            memset(state.buffer, 0, 512);
            state.show_preview = false;
        }
        return true;
    }

    if (key == GLFW_KEY_P) {
        struct file_entry *current = fm->current_dir->children[vr_list.selected];
        if (current->type == TYPE_FILE && get_mime_type(current->path, "text/")) {
            read_file_content(current->path, state.buffer, 512);
            state.show_preview = true;
        }
        return true;
    }

    return false;
}

bool handle_search_entry_key(int key) {
    if (key == GLFW_KEY_SLASH) {
        show_input(&search_input);
        return true;
    }

    if (search_input.is_visible) {
        if (key == GLFW_KEY_ESCAPE) {
            hide_input(&search_input);
        } else if (key == GLFW_KEY_BACKSPACE) {
            pop_from_input(&search_input);
        } else if (key == GLFW_KEY_ENTER) {
            // search current files
            emit_signal(EVENT_SEARCH, search_input.buffer);
            hide_input(&search_input);
        }
        return true;
    }

    if (rename_input.is_visible) {
        if (key == GLFW_KEY_ESCAPE) {
            hide_input(&rename_input);
        } else if (key == GLFW_KEY_BACKSPACE) {
            pop_from_input(&rename_input);
        } else if (key == GLFW_KEY_ENTER) {
            emit_signal(EVENT_RENAME, rename_input.buffer);
            hide_input(&rename_input);
        } else if (key == GLFW_KEY_LEFT) {
            move_cursor_left(&rename_input);
        } else if (key == GLFW_KEY_RIGHT) {
            move_cursor_right(&rename_input);
        }
        return true;
    }

    return false;
}

void character_callback(GLFWwindow *window, unsigned int codepoint) {
    if (search_input.is_visible) {
        if (codepoint == '/')
            return;
        append_to_input(&search_input, codepoint);
    } else if (rename_input.is_visible) {
        if (codepoint == '/')
            return;
        append_to_input(&rename_input, codepoint);
    }
}

void handle_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;

    // Global ESC to close info or preview
    if (state.show_info) {
        if (key == GLFW_KEY_ESCAPE) {
            state.show_info = false;
        }
        return;
    }

    if (handle_search_entry_key(key))
        return;

    if (search_input.is_visible || rename_input.is_visible) {
        return;
    }

    if (handle_option_list_key(&op_list, key))
        return;

    if (handle_file_entry_key(key))
        return;

    // Main view mode
    if (handle_global_key(key))
        return;
    if (handle_hr_list_key(key))
        return;
    if (handle_vr_list_key(key))
        return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

void op_list_option_selected(Option *option) {
    if (strcmp(option->title, "Information") == 0) {
        state.show_info = true;
    } else if (strcmp(option->title, "Rename") == 0) {
        fm->action_target_index = vr_list.selected; // Store target index
        set_buffer_input(&rename_input, fm->current_dir->children[vr_list.selected]->name);
        show_input(&rename_input);
    }
}

// Initialization of menu data
void initialize_menu_data() {
    init_horizontal_list(&hr_list);

    fm = create_file_manager(hr_list.items[hr_list.selected].path);

    // vr
    init_vertical_list(&vr_list);
    vr_list.get_screen_size = get_window_size;

    // options
    root.items[5].submenu = &options;
    options.parent = &root;

    op_list.on_item_selected = op_list_option_selected;

    op_list.root = &root;
    op_list.current = &root;
}

void file_manager_event_handler(EventType type, void *context, void *data) {
    FileManager *fm = (FileManager *)context;

    if (type == EVENT_NAVIGATE_TO_PATH) {
        NavigationData *nav_data = (NavigationData *)data;

        // Change directory
        if (nav_data->clear_history) {
            switch_directory(fm, nav_data->path);
        } else {
            change_directory(fm, nav_data->path);
        }

        // Notify about new directory content
        DirectoryData dir_data = {
            .selected = 0,
            .depth = fm->depth,
            .items = fm->current_dir->children,
            .current_path = fm->current_dir->path,
            .items_count = fm->current_dir->child_count,
        };
        emit_signal(EVENT_DIRECTORY_CONTENT_CHANGED, &dir_data);
    } else if (type == EVENT_ITEM_ACTIVATED) {
        int index = *(int *)data;
        struct file_entry *current = fm->current_dir->children[index];

        if (current->type == TYPE_DIRECTORY) {
            NavigationData nav_data = {.path = current->path, .selected_index = 0, .clear_history = false};
            emit_signal(EVENT_NAVIGATE_TO_PATH, &nav_data);
        } else {
            open_file(current->path);
        }
    } else if (type == EVENT_NAVIGATE_BACK) {
        // Only proceed if we can go back
        if (fm->depth <= 0)
            return;

        int selected = navigate_back(fm);

        DirectoryData dir_data = {
            .depth = fm->depth,
            .selected = selected,
            .items = fm->current_dir->children,
            .current_path = fm->current_dir->path,
            .items_count = fm->current_dir->child_count,
        };
        emit_signal(EVENT_DIRECTORY_CONTENT_CHANGED, &dir_data);
    } else if (type == EVENT_SEARCH) {
        char *search = (char *)data;

        int index = search_file_name(fm, search);
        if (index == -1)
            return;

        SelectionData sel_data = {.index = index};
        emit_signal(EVENT_VERTICAL_SELECTION_CHANGED, &sel_data);
    } else if (type == EVENT_RENAME) {
        char *new_name = (char *)data;

        if (!fm_rename(fm, new_name)) {
            printf("Couldn't rename file.\n");
            return;
        }

        DirectoryData dir_data = {
            .depth = fm->depth,
            .items = fm->current_dir->children,
            .selected = fm->action_target_index,
            .current_path = fm->current_dir->path,
            .items_count = fm->current_dir->child_count,
        };
        emit_signal(EVENT_DIRECTORY_CONTENT_CHANGED, &dir_data);
        fm->action_target_index = -1;
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
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
        return 1;
    }

    glfwSetWindowAttrib(window, GLFW_FLOATING, GL_TRUE);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, handle_key);
    glfwSetCharCallback(window, character_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Could not init glew.\n");
        return 1;
    }

    glfwSwapInterval(1);

    ui_create();

    init_ribbon();

    // Register a font
    if (register_font("sans", "./fonts/SpaceMonoNerdFont.ttf") < 0) {
        fprintf(stderr, "Failed to register font\n");
        ui_delete();
        return 1;
    }

    if (register_font("icon", "./fonts/feather.ttf") < 0) {
        fprintf(stderr, "Failed to register font icon\n");
        ui_delete();
        return 1;
    }

    state.theme = 2; // electric_blue
    srand(time(NULL));
    initialize_menu_data();

    ///////////////////////////////////////////

    // Horizontal list connections
    connect_signal(EVENT_HORIZONTAL_SELECTION_CHANGED, horizontal_list_event_handler, &hr_list);
    connect_signal(EVENT_DIRECTORY_CONTENT_CHANGED, horizontal_list_event_handler, &hr_list);

    // Vertical list connections
    connect_signal(EVENT_DIRECTORY_CONTENT_CHANGED, vertical_list_event_handler, &vr_list);
    connect_signal(EVENT_VERTICAL_SELECTION_CHANGED, vertical_list_event_handler, &vr_list);

    // File manager connections
    connect_signal(EVENT_NAVIGATE_TO_PATH, file_manager_event_handler, fm);
    connect_signal(EVENT_NAVIGATE_BACK, file_manager_event_handler, fm);
    connect_signal(EVENT_ITEM_ACTIVATED, file_manager_event_handler, fm);
    connect_signal(EVENT_SEARCH, file_manager_event_handler, fm);
    connect_signal(EVENT_RENAME, file_manager_event_handler, fm);

    ///////////////////////////////////////////

    // init
    emit_signal(EVENT_HORIZONTAL_SELECTION_CHANGED, &(SelectionData){.index = 0});

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();

        animation_update(current_time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        draw_background(width, height, state.theme);

        draw_ribbon(width, height, current_time);

        start_frame(width, height);

        if (!state.show_info) {
            draw_folder_path(&hr_list, fm->current_dir->path, 200, 160);
            draw_vertical_list(&vr_list, 180);
            draw_horizontal_menu(&hr_list, 180, 150);

            draw_text_preview(&state);

            draw_option_list(&op_list, &state);

            draw_input_field(&search_input, "Search", &state);
            draw_input_field(&rename_input, "Rename", &state);
        } else {
            draw_info(&vr_list, state.width, state.height);
        }

        end_frame();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_file_manager(fm);
    animation_clean();

    ui_delete();

    glfwTerminate();

    return 0;
}
