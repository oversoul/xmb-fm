#pragma once

#define MAX_CONNECTIONS 100
#define MAX_ARGS 10

typedef enum {
    // Navigation events
    EVENT_NAVIGATE_TO_PATH, // Navigate to a specific path
    EVENT_NAVIGATE_BACK,    // Navigate up one level

    // Selection events
    EVENT_HORIZONTAL_SELECTION_CHANGED, // Horizontal list selection changed
    EVENT_VERTICAL_SELECTION_CHANGED,   // Vertical list selection changed
    EVENT_ITEM_ACTIVATED,               // Item was activated (enter pressed)

    // State update events
    EVENT_DIRECTORY_CONTENT_CHANGED, // Directory contents changed

    OPTION_EVENT_OPEN_MENU,      // Open the menu
    OPTION_EVENT_CLOSE_MENU,     // Close current menu
    OPTION_EVENT_MOVE_SELECTION, // Move selection up/down
    OPTION_EVENT_SELECT_ITEM,    // Select current item
    OPTION_EVENT_OPEN_SUBMENU,   // Open a submenu

    MAX_EVENT_TYPES,
} EventType;

typedef struct {
    const char *path;
    int selected_index;
    bool clear_history;
} NavigationData;

typedef struct {
    int index;
} SelectionData;

typedef struct {
    char *current_path;
    void *items;
    int items_count;
    int depth;
    int selected;
} DirectoryData;

typedef struct {
    EventType type;
    void **data;
} Event;

typedef void (*EventCallback)(EventType type, void *context, void *data);

typedef struct {
    EventCallback callback;
    void *context;
} EventListener;

// Structure to represent a connection
#define MAX_LISTENERS 5

typedef struct {
    EventListener listeners[MAX_EVENT_TYPES][MAX_LISTENERS];
    int listener_counts[MAX_EVENT_TYPES];
} EventManager;

int connect_signal(EventType type, EventCallback callback, void *context);
void emit_signal(EventType type, void *data);
