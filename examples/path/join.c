/**
 * @file examples/path/join.c
 */

#include <stdio.h>

#include "path.h"

int main() {
    // Path existence
    printf("Path exists: %d\n", path_exists("data/mnist/"));

    // Normalization
    char* normalized = path_add_trailing_slash("data/mnist");
    printf("With trailing slash: %s\n", normalized);
    free(normalized);

    normalized = path_remove_trailing_slash("data/mnist/");
    printf("Without trailing slash: %s\n", normalized);
    free(normalized);

    normalized = path_remove_leading_slash("/data/mnist");
    printf("Without leading slash: %s\n", normalized);
    free(normalized);

    // Joining paths
    char* joined = path_join("data/mnist", "/training");
    printf("Joined path: %s\n", joined);
    free(joined);

    return 0;
}
