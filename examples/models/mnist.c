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
 *
 * Potential Options are:
 *    - He: Var(W) = mu + (2 / n)
 *    - Glorot and Bengio: Var(W) = mu + (2 / (n_in + n_out))
 */

#include <dirent.h>
#include <math.h>
#include <stb/stb_image.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    srand(42); // Seed for reproducibility
    return 0;
}
