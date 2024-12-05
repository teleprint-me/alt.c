/**
 * @file examples/path/entry.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <stdio.h>

#include "logger.h"
#include "path.h"

// Allocates directory entries
PathEntry* path_create_entry(const char* path, int current_depth, int max_depth) {
    if (!path_is_valid(path) || !path_is_directory(path) || current_depth > max_depth) {
        return NULL;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        LOG_ERROR("Failed to open directory '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    PathEntry* entry = malloc(sizeof(PathEntry));
    if (!entry) {
        closedir(dir);
        return NULL;
    }

    entry->info = NULL;
    entry->length = 0;

    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        char* entry_path = path_join(path, dir_entry->d_name);
        if (!entry_path) {
            LOG_ERROR("Failed to join path for '%s'.\n", dir_entry->d_name);
            continue;
        }

        PathInfo* info = path_create_info(entry_path);
        if (!info) {
            LOG_ERROR("Failed to retrieve metadata for '%s'.\n", entry_path);
            path_free_string(entry_path);
            continue;
        }

        // If it's a directory and within depth limit, recursively fetch sub-entries
        if (info->type == FILE_TYPE_DIRECTORY && current_depth < max_depth) {
            PathEntry* sub_entry = path_create_entry(entry_path, current_depth + 1, max_depth);
            if (sub_entry) {
                // Append sub-entry items to the current entry
                for (uint32_t i = 0; i < sub_entry->length; i++) {
                    PathInfo** new_info
                        = realloc(entry->info, sizeof(PathInfo*) * (entry->length + 1));
                    if (!new_info) {
                        path_free_info(sub_entry->info[i]);
                        continue;
                    }
                    entry->info = new_info;
                    entry->info[entry->length++] = sub_entry->info[i];
                }
                free(sub_entry->info);
                free(sub_entry);
            }
        }

        // Append current `info` to `entry->info`
        PathInfo** new_info = realloc(entry->info, sizeof(PathInfo*) * (entry->length + 1));
        if (!new_info) {
            path_free_info(info);
            path_free_string(entry_path);
            continue;
        }
        entry->info = new_info;
        entry->info[entry->length++] = info;

        path_free_string(entry_path);
    }

    closedir(dir);
    return entry;
}

// Frees a PathEntry structure
void path_free_entry(PathEntry* entry) {
    if (entry) {
        for (uint32_t i = 0; i < entry->length; i++) {
            path_free_info(entry->info[i]);
        }
        if (entry->info) {
            free(entry->info);
        }
        free(entry);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path> [max_depth]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* path = argv[1];
    int max_depth = (argc > 2) ? atoi(argv[2]) : 0;

    PathEntry* entry = path_create_entry(path, 0, max_depth);
    if (!entry) {
        fprintf(stderr, "Failed to list directory '%s'.\n", path);
        return EXIT_FAILURE;
    }

    // Print entries
    for (uint32_t i = 0; i < entry->length; i++) {
        const PathInfo* info = entry->info[i];
        printf("Path: %s, Type: %d, Size: %ld\n",
               info->path, info->type, info->size);
    }
    printf("Listed %u entries in directory '%s'.\n", entry->length, path);

    path_free_entry(entry);
    return EXIT_SUCCESS;
}
