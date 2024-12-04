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
        char* entry_path = path_join(path, entry->d_name);
        PathInfo*info = path_create_info(entry_path);
        if (!info) {
            // Could not read entry metadata with stat
            return EXIT_FAILURE;
        }

        printf("0x%7lx | ", info->inode);
        if (info->access | PATH_ACCESS_READ) {
            printf("r");
        }
        if (info->access | PATH_ACCESS_WRITE) {
            printf("w");
        }
        if (info->access | PATH_ACCESS_EXEC) {
            printf("x");
        }
        printf(" | ");
        printf("%s\n", entry->d_name); // Output entry name
        // cleanup
        path_free_info(info);
        path_free_string(entry_path);
    }

    // Clean up
    closedir(dir);
    return EXIT_SUCCESS;
}
