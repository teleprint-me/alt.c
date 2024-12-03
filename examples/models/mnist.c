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

#include "activation.h"
#include "random.h"

#define IMAGE_SIZE 28 * 28  // Flattened size of MNIST images

// Struct to hold a single image and label
typedef struct {
    float *pixels;
    int label;
} MNISTSample;

int main(int argc, char* argv[]) {
    srand(42); // Seed for reproducibility
    return 0;
}
