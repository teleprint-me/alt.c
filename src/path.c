/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/path.h
 *
 * @brief
 */

#include "path.h"

// Joins two paths and allocates memory for the result
char* path_join(const char* root_path, const char* sub_path) {
    int path_size = strlen(root_path) + strlen(sub_path) + 1;
    char* new_path = (char*) malloc(path_size);
    if (!new_path) {
        perror("Failed to allocate memory for path");
        return NULL;
    }
    strcpy(new_path, root_path);
    strcat(new_path, sub_path);
    return new_path;
}

// Frees a dynamically allocated path
void path_free(char* path) {
    if (path) {
        free(path);
    }
}

// Checks if a path exists
int path_exists(const char* path) {
    return access(path, F_OK) == 0;
}
