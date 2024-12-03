/**
 * @file examples/models/mnist.c
 *
 * @ref https://yann.lecun.com/exdb/mnist/
 * @ref https://github.com/myleott/mnist_png.git
 *
 * @paths
 *    - data/mnist/training
 *    - data/mnist/testing
 *
 * @note Normalized weight initialization for weights is fine.
 */

#include <dirent.h>
#include <math.h>
#include <pthread.h>
#include <stb/stb_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "activation.h"
#include "random.h"

#define IMAGE_SIZE 28 * 28 // Flattened size of MNIST images

// Struct to hold a single image and label
typedef struct {
    float* pixels;
    int label;
} MNISTSample;

// Joins two paths and allocates memory for the result
char* path_join(const char* root_path, const char* sub_path) {
    int path_size = strlen(root_path) + strlen(sub_path) + 1;
    char* new_path = (char*)malloc(path_size);
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

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path_to_mnist>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* root_path = argv[1];
    char* training_path = path_join(root_path, "/training");
    char* testing_path = path_join(root_path, "/testing");

    if (training_path && testing_path) {
        printf("Training Path: %s\n", training_path);
        printf("Testing Path: %s\n", testing_path);

        // Validate paths
        if (path_exists(training_path)) {
            printf("Training path exists!\n");
        } else {
            printf("Training path does not exist.\n");
        }

        if (path_exists(testing_path)) {
            printf("Testing path exists!\n");
        } else {
            printf("Testing path does not exist.\n");
        }
    } else {
        fprintf(stderr, "Failed to construct paths.\n");
        path_free(training_path);
        path_free(testing_path);
        return EXIT_FAILURE;
    }

    path_free(training_path);
    path_free(testing_path);

    return EXIT_SUCCESS;
}