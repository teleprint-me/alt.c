/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/random.c
 *
 * @brief Functions for initializing model weights.
 */

#include "interface/random.h"

void random_seed(uint32_t seed) {
    srand(seed);
}

// Linear initialization [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Initializes a vector, flat matrix, flat tensor, etc.
void random_linear_init_vector(float* vector, uint32_t width) {
    if (!(width > 0 && width < UINT32_MAX)) {
        return;
    }

    for (uint32_t i = 0; i < width; i++) {
        vector[i] = random_linear();
    }
}

void random_linear_init_matrix(float* matrix, uint32_t height, uint32_t width) {
    uint32_t size = height * width;
    if (!(size > 0 && size < UINT32_MAX)) {
        return;
    }

    for (uint32_t i = 0; i < size; i++) {
        matrix[i] = random_linear();
    }
}

// Uniform distribution
float random_uniform(float min, float max) {
    assert(max > min);
    return min + ((float) rand() / (float) RAND_MAX) * (max - min);
}

// Box–Muller transform
float random_gaussian(float mean, float stddev) {
    float u1 = random_linear();
    float u2 = random_linear();
    float z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float) M_PI * u2);
    return mean + z0 * stddev;
}

// He initialization
float random_kaiming_he(int32_t fan_in) {
    assert(fan_in > 0);
    return random_gaussian(0.0f, sqrtf(2.0f / (float) fan_in));
}

// Xavier and Glorot initialization
float random_xavier_glorot(int32_t fan_in, int32_t fan_out) {
    assert(fan_in > 0);
    assert(fan_out > 0);
    return random_gaussian(0.0f, sqrtf(2.0f / (float)(fan_in + fan_out)));
}
