/**
 * @file examples/path/join.c
 */

#include <stdio.h>

#include "path.h"

int main() {
    // Path existence
    printf("Path exists: %d\n", path_exists("data/mnist/"));

    // Normalization
    char* normalized = path_normalize("data/mnist", PATH_NORMALIZE_ADD_TRAILING_SLASH);
    printf("With trailing slash: %s\n", normalized);
    free(normalized);

    normalized = path_normalize("data/mnist/", PATH_NORMALIZE_REMOVE_TRAILING_SLASH);
    printf("Without trailing slash: %s\n", normalized);
    free(normalized);

    normalized = path_normalize("/data/mnist", PATH_NORMALIZE_ADD_LEADING_SLASH);
    printf("With leading slash: %s\n", normalized);
    free(normalized);

    normalized = path_normalize("/data/mnist", PATH_NORMALIZE_REMOVE_LEADING_SLASH);
    printf("Without leading slash: %s\n", normalized);
    free(normalized);

    // Joining paths
    char* joined = path_join("data/mnist", "/training");
    printf("Joined path: %s\n", joined);
    free(joined);

    return 0;
}
