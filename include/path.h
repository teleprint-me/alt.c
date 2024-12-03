/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/path.h
 *
 * @brief
 */

#ifndef ALT_PATH_H
#define ALT_PATH_H

// Required by GNUC because it is not defined by POSIX.
#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE // For d_type, e.g. DT_DIR, DT_REG, etc.
#endif // _DEFAULT_SOURCE

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

// Represents directory entries from a traversal
typedef struct PathEntity {
    struct dirent** entries; // Array of dirent pointers
    uint32_t length; // Number of entries
} PathEntity;

// Represents components of a split path
typedef struct PathSplit {
    char** parts; // Array of strings for path components
    uint32_t length; // Number of components
} PathSplit;

// Check if a path exists
bool path_exists(const char* path); // Returns true if the path exists, false otherwise

// Path manipulation functions
char* path_dir(const char* path); // Gets the directory part of a path (caller frees the result)
char* path_base(const char* path); // Gets the base name of a path (caller frees the result)
char* path_join(const char* root, const char* sub); // Joins two paths (caller frees the result)
void path_free_string(char* path); // Frees a string returned by path manipulation functions

// Path splitting functions
PathSplit* path_split(const char* path); // Splits a path into components
void path_free_split(PathSplit* split); // Frees a PathSplit structure

// Directory traversal functions
PathEntity* path_create_entity(void);
bool path_traverse(const char* base_path, PathEntity* entity, bool recursive); // Traverses a directory
void path_free_entity(PathEntity* entity); // Frees a PathEntity structure

#endif // ALT_PATH_H
