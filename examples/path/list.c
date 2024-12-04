/**
 * @file examples/path/list.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <dirent.h>
#include <stdio.h>

#include "logger.h"
#include "path.h"

// Buffer size for directory entries
#define READ_BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* path = argv[1];

    // Check if the path is a directory
    PathInfo* info = path_create_info(path);
    if (!info || info->type != FILE_TYPE_DIRECTORY) {
        fprintf(stderr, "Path '%s' is not a readable directory.\n", path);
        if (info) {
            path_free_info(info);
        }
        return EXIT_FAILURE;
    }

    // Open the directory
    DIR* dir = opendir(path);
    if (!dir) {
        LOG_ERROR("Failed to open directory '%s': %s", path, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Contents of directory '%s':\n", path);

    // Read the directory entries
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Do not skip files
        printf(" - %s\n", entry->d_name); // Output entry name
    }

    // Clean up
    closedir(dir);
    path_free_info(info);

    return EXIT_SUCCESS;
}
