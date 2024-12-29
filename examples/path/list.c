/**
 * @file examples/path/list.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <stdio.h>

#include "interface/logger.h"
#include "interface/path.h"

// Function to list contents of a directory with depth limit
void list_directory(const char* path, int current_depth, int max_depth) {
    if (current_depth > max_depth) return;

    // Open the directory
    DIR* dir = opendir(path);
    if (!dir) {
        LOG_ERROR("Failed to open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip `.` and `..`
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Join path
        char* entry_path = path_join(path, entry->d_name);

        // Retrieve metadata
        PathInfo* info = path_create_info(entry_path);
        if (!info) {
            LOG_ERROR("Failed to retrieve metadata for '%s'.\n", entry_path);
            path_free_string(entry_path);
            continue;
        }

        // Indent for recursive depth
        for (int i = 0; i < current_depth; i++) {
            printf("  ");
        }

        // Print metadata
        printf("0x%7lx\t", info->inode);
        if (info->type == FILE_TYPE_REGULAR) printf("file");
        if (info->type == FILE_TYPE_DIRECTORY) printf("dir");
        printf("\t");
        if (info->access & PATH_ACCESS_READ) printf("r");
        if (info->access & PATH_ACCESS_WRITE) printf("w");
        if (info->access & PATH_ACCESS_EXEC) printf("x");
        printf("\t%s\n", entry->d_name);

        // Recursively list subdirectories if within depth limit
        if (info->type == FILE_TYPE_DIRECTORY) {
            list_directory(entry_path, current_depth + 1, max_depth);
        }

        // Clean up
        path_free_info(info);
        path_free_string(entry_path);
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path> [max_depth]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* path = argv[1];
    int max_depth = (argc == 3) ? atoi(argv[2]) : 0; // Default depth is 0 if not specified

    printf("Contents of directory '%s':\n", path);
    list_directory(path, 0, max_depth);
    return EXIT_SUCCESS;
}
