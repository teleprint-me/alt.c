/**
 * @file examples/path/list.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include "logger.h"
#include "path.h"

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* path = argv[1];

    PathInfo* info = path_create_info(path);
    if (!info) {
        fprintf(stderr, "Failed to create PathInfo for '%s'.\n", path);
        return EXIT_FAILURE;
    }

    // if path is a directory and is readable open the path

    // use a pointer to traverse the directory path

    // output the contents for the directory regardless of file type

    path_print_info(info);
    path_free_info(info);

    return EXIT_SUCCESS;
}
