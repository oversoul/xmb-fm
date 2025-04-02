#pragma once

// Core file/directory entry structure
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef enum {
    TYPE_FILE = 0,
    TYPE_DIRECTORY = 1,
    TYPE_SYMLINK = 2,
} FileType;

typedef struct file_entry {
    char name[256];       // File/directory name
    char path[1024];      // Full path
    FileType type;        // 0: file, 1: directory, 2: symlink, etc.
    size_t size;          // File size in bytes
    time_t modified_time; // Last modified timestamp
    time_t access_time;   // Last access timestamp
    mode_t permissions;   // Unix-style permissions
    uid_t owner;          // Owner user ID
    gid_t group;          // Owner group ID

    // For directory navigation
    struct file_entry **children; // Array of child entries (for directories)
    size_t child_count;           // Number of children
    struct file_entry *parent;    // Parent directory

    // For UI purposes
    bool selected; // Selection state in UI
    bool expanded; // Expansion state (for directories)

    float x;
    float y;
    float zoom;
    float alpha;
    float label_alpha;
} FileEntry;

// File manager state
typedef struct file_manager {
    // Current navigation state
    FileEntry *current_dir;    // Currently displayed directory
    FileEntry *selected_entry; // Currently selected item

    // History for navigation
    FileEntry **history; // Directory history
    size_t history_size; // Size of history array
    size_t history_pos;  // Current position in history

    // View settings
    uint8_t sort_mode; // 0: name, 1: date, 2: size, etc.
    bool show_hidden;  // Whether to show hidden files
    bool reverse_sort; // Ascending/descending sort

    // UI state
    int scroll_offset; // Scroll position
    int view_width;    // View dimensions
    int view_height;

    // Optional: for multi-pane view
    struct file_manager *other_pane; // Pointer to other pane (for dual-pane)
} FileManager;

// Function prototypes for core operations
FileEntry *create_file_entry(const char *path);
void free_file_entry(FileEntry *entry, bool recursive);
int read_directory(FileEntry *dir);
int copy_entry(FileEntry *source, const char *dest_path);
int move_entry(FileEntry *source, const char *dest_path);
int delete_entry(FileEntry *entry);
int create_directory(const char *path);

void go_back(FileManager *fm);

// UI/Navigation functions
void change_directory(FileManager *fm, const char *path);
void select_entry(FileManager *fm, int index);
void refresh_current_dir(FileManager *fm);
void sort_entries(FileManager *fm);
void toggle_hidden_files(FileManager *fm);

FileManager *create_file_manager(const char *start_path);
void free_file_manager(FileManager *fm);
