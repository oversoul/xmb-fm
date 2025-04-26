#include "signal.h"
#include <stddef.h>
#include <stdio.h>

const char *event_types[] = {
    "EVENT_CHANGE_HORIZONTAL_INDEX",
    "EVENT_INCREASE_HORIZONTAL_DEPTH",
    "EVENT_DECREASE_HORIZONTAL_DEPTH",

    "EVENT_SWITCH_DIRECTORY",
    "EVENT_CHANGE_DIRECTORY",
    "EVENT_DIRECTORY_CHANGED",

    "EVENT_VERTICAL_LIST_INDEX_POPPED",
    "EVENT_VERTICAL_LIST_INDEX_CHANGED",
    "EVENT_VERTICAL_LIST_INDEX_VALIDATED",

    "EVENT_FM_OPEN_SELECTED_ITEM",
    "EVENT_FM_CLOSE_FOLDER",
};

static EventManager manager;

int connect_signal(EventType type, EventCallback callback, void *context) {
    if (!callback || type >= MAX_EVENT_TYPES)
        return 0;

    int count = manager.listener_counts[type];
    if (count >= MAX_LISTENERS)
        return 1; // No space for more listeners

    // Add new listener
    manager.listeners[type][count].callback = callback;
    manager.listeners[type][count].context = context;
    manager.listener_counts[type]++;
    return 1;
}

int disconnect_signal(EventType type, EventCallback callback) {
    if (!callback || type >= MAX_EVENT_TYPES)
        return 0;

    int count = manager.listener_counts[type];

    // Find the listener with the matching callback
    for (size_t i = 0; i < count; ++i) {
        if (manager.listeners[type][i].callback == callback) {
            // Remove by shifting the rest down
            for (size_t j = i; j < count - 1; ++j) {
                manager.listeners[type][j] = manager.listeners[type][j + 1];
            }
            manager.listener_counts[type]--;
            break;
        }
    }
    return 1;
}

void emit_signal(EventType type, void *data) {
    if (type >= MAX_EVENT_TYPES)
        return;

    // printf("SIGNAL: %s\n", event_types[type]);

    int count = manager.listener_counts[type];

    // Call all registered callbacks for this event type
    for (size_t i = 0; i < count; ++i) {
        EventListener *listener = &manager.listeners[type][i];
        listener->callback(type, listener->context, data);
    }
}
