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

#include <math.h>
#include <pthread.h>
#include <stb/stb_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "activation.h"
#include "path.h"
#include "random.h"

#define IMAGE_SIZE 28 * 28 // Flattened size of MNIST images

// Struct to hold a single image and label
typedef struct {
    float* pixels;
    int label;
} MNISTSample;

MNISTSample* create_mnist_samples(uint32_t count) {
    MNISTSample* samples = malloc(sizeof(MNISTSample) * count);
    if (!samples) {
        fprintf(stderr, "Failed to allocate MNIST samples.\n");
        return NULL;
    }
    for (uint32_t i = 0; i < count; i++) {
        samples[i].pixels = malloc(sizeof(float) * IMAGE_SIZE);
        samples[i].label = -1;
    }
    return samples;
}

void free_mnist_samples(MNISTSample* samples, uint32_t count) {
    if (!samples) {
        return;
    }
    for (uint32_t i = 0; i < count; i++) {
        free(samples[i].pixels);
    }
    free(samples);
}

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path_to_mnist>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* training_path = path_join(argv[1], "/training");

    if (!path_exists(training_path)) {
        fprintf(stderr, "Training path does not exist!\n");
        return EXIT_FAILURE;
    }

    // path_traverse(training_path, training_entity, true);
    // path_traverse(testing_path, testing_entity, true);

    path_free_string((char*) training_path);

    return EXIT_SUCCESS;
}
