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

// stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// libc
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// alt
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
    if (samples) {
        for (uint32_t i = 0; i < count; i++) {
            free(samples[i].pixels);
        }
        free(samples);
    }
}

uint32_t load_mnist_samples(const char* path, MNISTSample* samples, uint32_t max_samples) {
    PathEntry* entry
        = path_create_entry(path, 0, 1); // Depth 1: labels are in immediate subdirectories
    if (!entry) {
        fprintf(stderr, "Failed to traverse path '%s'.\n", path);
        return 0;
    }

    uint32_t sample_count = 0;

    for (uint32_t i = 0; i < entry->length && sample_count < max_samples; i++) {
        PathInfo* info = entry->info[i];

        // Skip non-file entries
        if (info->type != FILE_TYPE_REGULAR) {
            continue;
        }

        // Parse label from parent directory name
        char* parent_dir = path_dirname(info->path);
        int label = atoi(strrchr(parent_dir, '/') + 1); // Assume label is the directory name
        free(parent_dir);

        // Load image
        int width, height, channels;
        unsigned char* image_data
            = stbi_load(info->path, &width, &height, &channels, 1); // Grayscale
        if (!image_data) {
            fprintf(stderr, "Failed to load image '%s'.\n", info->path);
            continue;
        }

        // Ensure image dimensions match MNIST (28x28)
        if (width != 28 || height != 28) {
            fprintf(stderr, "Invalid image dimensions for '%s'.\n", info->path);
            stbi_image_free(image_data);
            continue;
        }

        // Convert image data to float and normalize to [0, 1]
        for (int j = 0; j < IMAGE_SIZE; j++) {
            samples[sample_count].pixels[j] = image_data[j] / 255.0f;
        }
        samples[sample_count].label = label;

        stbi_image_free(image_data);
        sample_count++;
    }

    path_free_entry(entry);
    return sample_count;
}

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path_to_mnist>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* training_path = path_join(argv[1], "training");
    if (!path_exists(training_path)) {
        fprintf(stderr, "Training path does not exist!\n");
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    const uint32_t max_samples = 60000; // Adjust as needed
    MNISTSample* samples = create_mnist_samples(max_samples);
    if (!samples) {
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    printf("Loading MNIST training data from '%s'...\n", training_path);
    uint32_t loaded_samples = load_mnist_samples(training_path, samples, max_samples);

    printf("Loaded %u samples.\n", loaded_samples);

    // Cleanup
    free_mnist_samples(samples, max_samples);
    path_free_string(training_path);

    return EXIT_SUCCESS;
}
