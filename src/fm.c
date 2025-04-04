#include "fm.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

// Create a new file entry from path
FileEntry *create_file_entry(const char *path) {
    FileEntry *entry = (FileEntry *)malloc(sizeof(FileEntry));
    if (!entry)
        return NULL;

    memset(entry, 0, sizeof(FileEntry));
    entry->y = 0;

    // Get filename from path
    const char *name = strrchr(path, '/');
    if (name)
        strncpy(entry->name, name + 1, sizeof(entry->name) - 1);
    else
        strncpy(entry->name, path, sizeof(entry->name) - 1);

    strncpy(entry->path, path, sizeof(entry->path) - 1);

    // Get file info
    struct stat st;
    if (stat(path, &st) == 0) {
        entry->size = st.st_size;
        entry->modified_time = st.st_mtime;
        entry->access_time = st.st_atime;
        entry->permissions = st.st_mode;
        entry->owner = st.st_uid;
        entry->group = st.st_gid;

        // Determine file type
        if (S_ISDIR(st.st_mode))
            entry->type = 1; // Directory
        else if (S_ISLNK(st.st_mode))
            entry->type = 2; // Symlink
        else
            entry->type = 0; // Regular file
    }

    entry->children = NULL;
    entry->child_count = 0;
    entry->parent = NULL;

    return entry;
}

// Free a file entry and optionally its children
void free_file_entry(FileEntry *entry, bool recursive) {
    if (!entry)
        return;

    if (recursive && entry->children) {
        for (size_t i = 0; i < entry->child_count; i++) {
            free_file_entry(entry->children[i], true);
        }
        free(entry->children);
    }

    free(entry);
}

// Compare function for sorting by name
static int compare_by_name(const void *a, const void *b) {
    FileEntry *entry_a = *(FileEntry **)a;
    FileEntry *entry_b = *(FileEntry **)b;

    // Directories first, then files
    if (entry_a->type == 1 && entry_b->type != 1)
        return -1;
    if (entry_a->type != 1 && entry_b->type == 1)
        return 1;

    return strcasecmp(entry_a->name, entry_b->name);
}

// Compare function for sorting by size
static int compare_by_size(const void *a, const void *b) {
    FileEntry *entry_a = *(FileEntry **)a;
    FileEntry *entry_b = *(FileEntry **)b;

    // Directories first, then files
    if (entry_a->type == 1 && entry_b->type != 1)
        return -1;
    if (entry_a->type != 1 && entry_b->type == 1)
        return 1;

    if (entry_a->size < entry_b->size)
        return -1;
    if (entry_a->size > entry_b->size)
        return 1;
    return 0;
}

// Compare function for sorting by date
static int compare_by_date(const void *a, const void *b) {
    FileEntry *entry_a = *(FileEntry **)a;
    FileEntry *entry_b = *(FileEntry **)b;

    if (entry_a->modified_time < entry_b->modified_time)
        return -1;
    if (entry_a->modified_time > entry_b->modified_time)
        return 1;
    return 0;
}

// Read directory contents
int read_directory(FileEntry *dir) {
    if (!dir)
        return -1;

    // Free existing children if any
    if (dir->children) {
        for (size_t i = 0; i < dir->child_count; i++) {
            free_file_entry(dir->children[i], true);
        }
        free(dir->children);
        dir->children = NULL;
        dir->child_count = 0;
    }

    DIR *d = opendir(dir->path);
    if (!d) {
        fprintf(stderr, "Couldn't open directory: %s\n", dir->path);
        return -1;
    }

    // Count entries first to allocate array
    struct dirent *entry;
    size_t count = 0;

    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (entry->d_name[0] == '.')
            continue;

        count++;
    }

    // Allocate array
    dir->children = (FileEntry **)malloc(count * sizeof(FileEntry *));
    if (!dir->children) {
        closedir(d);
        return -1;
    }

    // Rewind and read entries
    rewinddir(d);
    size_t index = 0;

    while ((entry = readdir(d)) != NULL && index < count) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (entry->d_name[0] == '.')
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir->path, entry->d_name);

        FileEntry *file_entry = create_file_entry(full_path);
        if (file_entry) {
            file_entry->parent = dir;
            dir->children[index++] = file_entry;
        }
    }

    dir->child_count = index;
    closedir(d);

    return 0;
}

// Copy a file or directory
int copy_entry(FileEntry *source, const char *dest_path) {
    if (!source || !dest_path)
        return -1;

    char target[1024];
    snprintf(target, sizeof(target), "%s/%s", dest_path, source->name);

    if (source->type == 0) { // Regular file
        FILE *src = fopen(source->path, "rb");
        if (!src)
            return -1;

        FILE *dst = fopen(target, "wb");
        if (!dst) {
            fclose(src);
            return -1;
        }

        char buffer[8192];
        size_t bytes;

        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytes, dst);
        }

        fclose(src);
        fclose(dst);

        // Copy permissions
        chmod(target, source->permissions);
        chown(target, source->owner, source->group);

        return 0;
    } else if (source->type == 1) { // Directory
        // Create target directory
        mkdir(target, source->permissions);
        chown(target, source->owner, source->group);

        // Make sure children are loaded
        if (!source->children) {
            read_directory(source);
        }

        // Recursively copy children
        for (size_t i = 0; i < source->child_count; i++) {
            copy_entry(source->children[i], target);
        }

        return 0;
    }

    return -1; // Unsupported file type
}

// Move a file or directory
int move_entry(FileEntry *source, const char *dest_path) {
    if (!source || !dest_path)
        return -1;

    char target[1024];
    snprintf(target, sizeof(target), "%s/%s", dest_path, source->name);

    // Try simple rename first (works if on same filesystem)
    if (rename(source->path, target) == 0) {
        return 0;
    }

    // If rename fails, copy and delete
    if (copy_entry(source, dest_path) == 0) {
        return delete_entry(source);
    }

    return -1;
}

// Delete a file or directory
int delete_entry(FileEntry *entry) {
    if (!entry)
        return -1;

    if (entry->type == 0) { // Regular file
        if (unlink(entry->path) != 0) {
            return -1;
        }
    } else if (entry->type == 1) { // Directory
        // Make sure children are loaded
        if (!entry->children) {
            read_directory(entry);
        }

        // Recursively delete children
        for (size_t i = 0; i < entry->child_count; i++) {
            delete_entry(entry->children[i]);
        }

        // Delete empty directory
        if (rmdir(entry->path) != 0) {
            return -1;
        }
    }

    return 0;
}

// Create a new directory
int create_directory(const char *path) {
    if (!path)
        return -1;

    // Create with standard permissions (755)
    if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        return -1;
    }

    return 0;
}

// Create a new file manager
FileManager *create_file_manager(const char *start_path) {
    FileManager *fm = (FileManager *)malloc(sizeof(FileManager));
    if (!fm)
        return NULL;

    memset(fm, 0, sizeof(FileManager));

    // Set default sorting
    fm->sort_mode = 0; // Sort by name
    fm->show_hidden = false;
    fm->reverse_sort = false;

    // Initialize history
    fm->history_size = 32;
    fm->history = (FileEntry **)malloc(fm->history_size * sizeof(FileEntry *));
    fm->history_pos = 0;

    if (!fm->history) {
        free(fm);
        return NULL;
    }

    // Start at given path or current directory
    const char *path = start_path ? start_path : ".";
    fm->current_dir = create_file_entry(path);

    if (!fm->current_dir) {
        free(fm->history);
        free(fm);
        return NULL;
    }

    // Load directory contents
    read_directory(fm->current_dir);
    fm->history[0] = fm->current_dir;

    return fm;
}

// Clean up file manager
void free_file_manager(FileManager *fm) {
    if (!fm)
        return;

    if (fm->history) {
        for (size_t i = 0; i < fm->history_pos; i++) {
            if (fm->history[i] && fm->history[i] != fm->current_dir) {
                free_file_entry(fm->history[i], true);
            }
        }
        free(fm->history);
    }

    if (fm->current_dir) {
        free_file_entry(fm->current_dir, true);
    }

    free(fm);
}

// Change current directory
void change_directory(FileManager *fm, const char *path) {
    if (!fm || !path)
        return;

    FileEntry *new_dir = create_file_entry(path);
    if (!new_dir || new_dir->type != 1) {
        free_file_entry(new_dir, false);
        return;
    }

    if (read_directory(new_dir) != 0) {
        free_file_entry(new_dir, true);
        return;
    }

    // Add to history
    if (fm->history_pos < fm->history_size - 1) {
        fm->history_pos++;
        fm->history[fm->history_pos] = new_dir;
    } else {
        // History full, shift and add
        free_file_entry(fm->history[0], true);
        for (size_t i = 0; i < fm->history_size - 1; i++) {
            fm->history[i] = fm->history[i + 1];
        }
        fm->history[fm->history_size - 1] = new_dir;
    }

    fm->current_dir = new_dir;
    fm->selected_entry = NULL;

    sort_entries(fm);
}

// Refresh current directory contents
void refresh_current_dir(FileManager *fm) {
    if (!fm || !fm->current_dir)
        return;

    // Remember selected entry name to restore selection after refresh
    char selected_name[256] = {0};
    if (fm->selected_entry) {
        strncpy(selected_name, fm->selected_entry->name, sizeof(selected_name) - 1);
    }

    read_directory(fm->current_dir);
    sort_entries(fm);
}

// Sort entries according to current sort mode
void sort_entries(FileManager *fm) {
    if (!fm || !fm->current_dir || !fm->current_dir->children)
        return;

    // Choose comparison function based on sort mode
    int (*compare_func)(const void *, const void *);

    switch (fm->sort_mode) {
    case 1: // By date
        compare_func = compare_by_date;
        break;
    case 2: // By size
        compare_func = compare_by_size;
        break;
    case 0: // By name (default)
    default:
        compare_func = compare_by_name;
        break;
    }

    // Sort entries
    qsort(fm->current_dir->children, fm->current_dir->child_count, sizeof(FileEntry *), compare_func);

    // Reverse if needed
    if (fm->reverse_sort && fm->current_dir->child_count > 1) {
        for (size_t i = 0; i < fm->current_dir->child_count / 2; i++) {
            FileEntry *temp = fm->current_dir->children[i];
            fm->current_dir->children[i] = fm->current_dir->children[fm->current_dir->child_count - 1 - i];
            fm->current_dir->children[fm->current_dir->child_count - 1 - i] = temp;
        }
    }
}

// Toggle showing hidden files
void toggle_hidden_files(FileManager *fm) {
    if (!fm)
        return;

    fm->show_hidden = !fm->show_hidden;
    refresh_current_dir(fm);
}

// Go back in history
void go_back(FileManager *fm) {
    if (!fm || fm->history_pos <= 0)
        return;

    fm->history_pos--;
    fm->current_dir = fm->history[fm->history_pos];
    fm->selected_entry = NULL;

    refresh_current_dir(fm);
}

// Go forward in history
void go_forward(FileManager *fm) {
    if (!fm || fm->history_pos >= fm->history_size - 1 || !fm->history[fm->history_pos + 1])
        return;

    fm->history_pos++;
    fm->current_dir = fm->history[fm->history_pos];
    fm->selected_entry = NULL;

    refresh_current_dir(fm);
}

// Simple function to display file manager contents to console (for testing)
void display_directory(FileManager *fm) {
    if (!fm || !fm->current_dir) {
        printf("No directory to display\n");
        return;
    }

    printf("Current directory: %s\n", fm->current_dir->path);
    printf("--------------------------------------------------\n");

    for (size_t i = 0; i < fm->current_dir->child_count; i++) {
        FileEntry *entry = fm->current_dir->children[i];
        char type_char = entry->type == 1 ? 'd' : (entry->type == 2 ? 'l' : 'f');

        printf("%c %s%s (%zu bytes)\n", type_char, entry->name, entry->type == 1 ? "/" : "", entry->size);
    }

    printf("--------------------------------------------------\n");
}
