#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef enum {
    TYPE_FILE = 0,
    TYPE_DIRECTORY = 1,
    TYPE_SYMLINK = 2,
} FileType;

typedef enum {
    SortByName = 0,
    SortByDate = 1,
    SortBySize = 2,
} SortMode;

typedef struct file_entry {
    char name[256];
    char path[1024];
    FileType type;

    struct file_entry **children;
    size_t child_count;

    struct file_entry *parent;

    size_t size;          // File size in bytes
    time_t access_time;   // Last access timestamp
    mode_t permissions;   // Unix-style permissions
    time_t modified_time; // Last modified timestamp

    float x;
    float y;
    float zoom;
    float alpha;
    float label_alpha;
} FileEntry;

typedef struct file_manager {
    int depth;
    FileEntry *current_dir;

    // View settings
    bool show_hidden;
    bool reverse_sort;
    SortMode sort_mode;
} FileManager;

FileManager *create_file_manager(const char *path);
void free_file_manager(FileManager *fm);

void change_directory(FileManager *fm, const char *path);
int navigate_back(FileManager *fm);
void switch_directory(FileManager *fm, const char *path);

bool get_mime_type(const char *path, const char *test);
void open_file(const char *path);
void read_file_content(const char *filename, char *buffer, size_t len);
int search_file_name(FileManager *fm, const char *keyword);
