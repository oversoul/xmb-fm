#include "fm.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include <magic.h>
#include <fcntl.h>

static bool show_file(struct dirent *entry) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        return false;
    if (entry->d_name[0] == '.')
        return false;
    return true;
}

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

static int compare_by_size(const void *a, const void *b) {
    FileEntry *entry_a = *(FileEntry **)a;
    FileEntry *entry_b = *(FileEntry **)b;

    // Directories first, then files
    if (entry_a->type == TYPE_DIRECTORY && entry_b->type != TYPE_DIRECTORY)
        return -1;
    if (entry_a->type != TYPE_DIRECTORY && entry_b->type == TYPE_DIRECTORY)
        return 1;

    if (entry_a->size < entry_b->size)
        return -1;
    if (entry_a->size > entry_b->size)
        return 1;
    return 0;
}

static int compare_by_date(const void *a, const void *b) {
    FileEntry *entry_a = *(FileEntry **)a;
    FileEntry *entry_b = *(FileEntry **)b;

    if (entry_a->modified_time < entry_b->modified_time)
        return -1;
    if (entry_a->modified_time > entry_b->modified_time)
        return 1;
    return 0;
}

static void sort_entries(FileManager *fm) {
    if (!fm || !fm->current_dir || !fm->current_dir->children)
        return;

    int (*compare_func)(const void *, const void *);

    switch (fm->sort_mode) {
    case SortByDate:
        compare_func = compare_by_date;
        break;
    case SortBySize:
        compare_func = compare_by_size;
        break;
    case SortByName:
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

static FileEntry *create_file_entry(const char *path) {
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
        entry->permissions = st.st_mode;
        entry->access_time = st.st_atime;
        entry->modified_time = st.st_mtime;

        if (S_ISDIR(st.st_mode))
            entry->type = TYPE_DIRECTORY;
        else if (S_ISLNK(st.st_mode))
            entry->type = TYPE_SYMLINK;
        else
            entry->type = TYPE_FILE;
    }

    entry->children = NULL;
    entry->child_count = 0;
    entry->parent = NULL;

    return entry;
}

static int read_directory(FileEntry *dir) {
    if (!dir)
        return -1;

    if (dir->children) {
        for (size_t i = 0; i < dir->child_count; ++i) {
            free(dir->children[i]);
        }

        free(dir->children);
    }

    DIR *d = opendir(dir->path);
    if (!d) {
        fprintf(stderr, "Couldn't open directory: %s\n", dir->path);
        return -1;
    }

    struct dirent *entry;
    size_t count = 0;

    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (!show_file(entry))
            continue;

        count++;
    }

    dir->children = (FileEntry **)malloc(count * sizeof(FileEntry *));
    if (!dir->children) {
        closedir(d);
        return -1;
    }

    // Rewind and read entries
    rewinddir(d);
    size_t index = 0;

    while ((entry = readdir(d)) != NULL && index < count) {
        if (!show_file(entry))
            continue;

        char full_path[1024];
        if (strcmp(dir->path, "/") == 0) {
            snprintf(full_path, sizeof(full_path), "/%s", entry->d_name);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", dir->path, entry->d_name);
        }

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

static int find_index_of(FileManager *fm, const char *path, int default_index) {
    for (size_t i = 0; i < fm->current_dir->child_count; ++i) {
        if (strcmp(fm->current_dir->children[i]->path, path) == 0) {
            return i;
        }
    }

    return default_index;
}

static void free_file_entry(FileEntry *entry) {
    for (size_t i = 0; i < entry->child_count; ++i) {
        free(entry->children[i]);
    }

    free(entry->children);
    free(entry);
}

static void free_history(FileManager *fm) {
    FileEntry *entry = fm->current_dir;
    while (entry) {
        FileEntry *parent = entry->parent;
        free_file_entry(entry);
        entry = parent;
    }
}

FileManager *create_file_manager(const char *path) {
    FileManager *fm = (FileManager *)malloc(sizeof(FileManager));
    if (!fm)
        return NULL;

    memset(fm, 0, sizeof(FileManager));

    fm->sort_mode = SortByName;
    fm->show_hidden = false; // unimplemented
    fm->reverse_sort = false;

    fm->current_dir = create_file_entry(path);
    fm->current_dir->parent = NULL;
    read_directory(fm->current_dir);
    sort_entries(fm);

    return fm;
}

void change_directory(FileManager *fm, const char *path) {
    FileEntry *parent = fm->current_dir;

    fm->current_dir = create_file_entry(path);
    fm->current_dir->parent = parent;
    fm->depth++;
    read_directory(fm->current_dir);
    sort_entries(fm);
}

void switch_directory(FileManager *fm, const char *path) {
    free_history(fm);

    fm->current_dir = create_file_entry(path);
    fm->current_dir->parent = NULL;
    fm->depth = 0;
    read_directory(fm->current_dir);
    sort_entries(fm);
}

int navigate_back(FileManager *fm) {
    if (fm->current_dir->parent == NULL)
        return -1;

    FileEntry *old = fm->current_dir;

    fm->current_dir = fm->current_dir->parent;
    read_directory(fm->current_dir);
    sort_entries(fm);
    fm->depth--;

    int index = find_index_of(fm, old->path, 0);

    free_file_entry(old);
    return index;
}

void free_file_manager(FileManager *fm) {
    free_history(fm);
    free(fm);
}

bool get_mime_type(const char *path, const char *test) {
    bool ret = false;
    const char *mimetype;
    magic_t magic;

    if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
        fprintf(stderr, "Error opening libmagic.\n");
        return ret;
    }
    if (magic_load(magic, NULL) == -1) {
        fprintf(stderr, "Error magic_load.\n");
        magic_close(magic);
        return ret;
    }
    if ((mimetype = magic_file(magic, path)) == NULL) {
        fprintf(stderr, "Error getting mimetype.\n");
        magic_close(magic);
        return ret;
    }

    ret = strstr(mimetype, test) != NULL;

    magic_close(magic);
    return ret;
}

static void xdg_open(const char *str) {
    if (fork() == 0) {
        // redirect stdout and stderr to null
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execl("/usr/bin/xdg-open", "/usr/bin/xdg-open", str, (char *)0);
    }
}

void open_file(const char *path) {
    if (!access("/usr/bin/xdg-open", X_OK)) {
        xdg_open(path);
    } else {
        fprintf(stderr, "no xdg-open.\n");
    }
}

void read_file_content(const char *filename, char *buffer, size_t len) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "File couldn't be opened.\n");
        return;
    }

    size_t bytesRead = fread(buffer, 1, len - 1, file);
    buffer[bytesRead] = '\0';
    fclose(file);
}

int search_file_name(FileManager *fm, const char *keyword) {
    for (size_t i = 0; i < fm->current_dir->child_count; i++) {
        struct file_entry *entry = fm->current_dir->children[i];
        if (entry == NULL)
            continue;

        const char *name = entry->name;
        const char *key = keyword;
        while (*name) {
            const char *n = name;
            const char *k = key;
            while (*n && *k && (tolower(*n) == tolower(*k))) {
                n++;
                k++;
            }
            if (*k == '\0') // Found a match
                return i;
            name++;
        }
    }

    return -1;
}
