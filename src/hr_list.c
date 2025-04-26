#include "hr_list.h"
#include "animation.h"
#include "signal.h"
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

HrItem horizontalItems[10];

void init_horizontal_list(HorizontalList *hr_list) {
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    // Users category
    strcpy(horizontalItems[0].title, "Home");
    strcpy(horizontalItems[0].path, homedir);
    strcpy(horizontalItems[0].icon, "\ue977");

    // Settings category
    strcpy(horizontalItems[1].title, "Desktop");
    sprintf(horizontalItems[1].path, "%s/%s", homedir, "Desktop");
    strcpy(horizontalItems[1].icon, "\ue9b7");

    // Add more categories like in the Vue code
    strcpy(horizontalItems[2].title, "Documents");
    sprintf(horizontalItems[2].path, "%s/%s", homedir, "Documents");
    strcpy(horizontalItems[2].icon, "\ue909");

    // Songs category
    strcpy(horizontalItems[3].title, "Downloads");
    sprintf(horizontalItems[3].path, "%s/%s", homedir, "Downloads");
    strcpy(horizontalItems[3].icon, "\ue95f");

    // Movies category
    strcpy(horizontalItems[4].title, "Pictures");
    sprintf(horizontalItems[4].path, "%s/%s", homedir, "Pictures");
    strcpy(horizontalItems[4].icon, "\ue978");

    // Games category
    strcpy(horizontalItems[5].title, "Public");
    sprintf(horizontalItems[5].path, "%s/%s", homedir, "Public");
    strcpy(horizontalItems[5].icon, "\ueA07");

    // Network category
    strcpy(horizontalItems[6].title, "Videos");
    sprintf(horizontalItems[6].path, "%s/%s", homedir, "Videos");
    strcpy(horizontalItems[6].icon, "\ue94d");

    // Friends category
    strcpy(horizontalItems[7].title, "File System");
    strcpy(horizontalItems[7].path, "/");
    strcpy(horizontalItems[7].icon, "\ue958");

    // Initialize animation state
    hr_list->items = horizontalItems;
    hr_list->items_count = 8;
}

void horizontal_list_event_handler(EventType type, void *context, void *data) {
    HorizontalList *list = (HorizontalList *)context;

    if (type == EVENT_HORIZONTAL_SELECTION_CHANGED) {
        SelectionData *sel_data = (SelectionData *)data;

        animation_remove_by_tag(HorizontalListTag);
        update_horizontal_list(list);

        // Navigate to the selected path
        NavigationData nav_data = {
            .selected_index = 0,
            .clear_history = true,
            .path = list->items[sel_data->index].path,
        };
        emit_signal(EVENT_NAVIGATE_TO_PATH, &nav_data);
    } else if (type == EVENT_DIRECTORY_CONTENT_CHANGED) {
        DirectoryData *dir_data = (DirectoryData *)data;

        // Update depth as needed
        list->depth = dir_data->depth;

        // Update UI
        animation_remove_by_tag(HorizontalListTag);
        update_horizontal_list(list);
    }
}

void update_horizontal_list(HorizontalList *hr_list) {
    float offset = 150;
    float target = hr_list->selected * offset;

    if (hr_list->depth > 0)
        target += 50;

    animation_push(0.2, target, &hr_list->scroll, HorizontalListTag);
}
