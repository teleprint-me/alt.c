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
#define _DEFAULT_SOURCE // For d_type, e.g. DT_DIR, DT_REG, etc.

#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

// Joins two paths and allocates memory for the result
char* path_join(const char* prefix, const char* suffix);

// Frees a dynamically allocated path
void path_free(char* path);

// Checks if a path exists
int path_exists(const char* path);

#endif // ALT_PATH_H
