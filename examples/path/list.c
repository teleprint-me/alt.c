/**
 * @file examples/path/list.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <dirent.h>
#include <stdio.h>

#include "logger.h"
#include "path.h"

// Function to list contents of a directory
void list_directory(const char* path, int depth) {
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
        for (int i = 1; i < depth; i++) {
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

        // Recursively list subdirectories
        if (depth > 0 && info->type == FILE_TYPE_DIRECTORY) {
            list_directory(entry_path, depth + 1);
        }

        // Clean up
        path_free_info(info);
        path_free_string(entry_path);
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (!(argc > 1) || !argv[1]) {
        fprintf(stderr, "Usage: %s <path> <depth>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* path = argv[1];

    // get the depth if provided
    int depth = 0;
    if (argc == 3) {
        depth = atoi(argv[2]);
    }

    printf("Contents of directory '%s':\n", argv[1]);
    list_directory(path, depth);
    return EXIT_SUCCESS;
}
