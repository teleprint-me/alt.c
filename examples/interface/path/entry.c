/**
 * @file examples/path/entry.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <stdio.h>

#include "interface/logger.h"
#include "interface/path.h"

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
        printf("Path: %s, Type: %d, Size: %ld\n", info->path, info->type, info->size);
    }
    printf("Listed %u entries in directory '%s'.\n", entry->length, path);

    path_free_entry(entry);
    return EXIT_SUCCESS;
}
