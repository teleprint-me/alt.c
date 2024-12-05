/**
 * @file examples/path/base.c
 *
 * @brief Scratchpad for experimenting with path objects.
 */

#include <stdio.h>

#include "path.h"

int main() {
    const char* test_path = "data/mnist/training/0/1.png";

    char* dirname = path_dirname(test_path);
    char* basename = path_basename(test_path);

    printf("Path: %s\n", test_path);
    printf("Dirname: %s\n", dirname);
    printf("Basename: %s\n", basename);

    path_free_string(dirname);
    path_free_string(basename);

    return 0;
}
