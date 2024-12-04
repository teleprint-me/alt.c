/**
 * @file examples/scratchpad/path_traverse.c
 */

#include <stdio.h>

#include "path.h"

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get the base path
    char* base_path = argv[1];
    if (!base_path) {
        return EXIT_FAILURE;
    }

    // Validate the path exists
    if (!path_exists(base_path)) {
        fprintf(stderr, "Training path does not exist!\n");
        return EXIT_FAILURE;
    }

    // Create the path entity
    PathEntity* entity = path_create_entity();
    if (!entity) {
        fprintf(stderr, "Failed to allocate PathEntity\n");
        return EXIT_FAILURE;
    }

    // Recursively traverse the path with the given entity
    printf("Loading path entities...\n");
    path_traverse(base_path, entity, true);

    // Output traversal results
    printf("Traversing %d entries.\n", entity->length);
    for (uint32_t i = 0; i < entity->length; i++) {
        struct dirent* entry = entity->entries[i];
        printf("name: %s, length: %u, type: %u\n", entry->d_name, entry->d_reclen, entry->d_type);
    }

    // Cleanup
    path_free_entity(entity);
    return EXIT_SUCCESS;
}
