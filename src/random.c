/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/random.c
 *
 * @brief Functions for initializing model weights.
 */

#include "random.h"

void random_seed(uint32_t seed) {
    srand(seed);
}

// Linear initialization [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
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
    float z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
    return mean + z0 * stddev;
}

// He initialization
float random_he(int32_t fan_in) {
    assert(fan_in > 0);
    return random_gaussian(0.0f, sqrtf(2.0f / (float) fan_in));
}

// Xavier and Glorot initialization
float random_glorot(int32_t fan_in, int32_t fan_out) {
    assert(fan_in > 0);
    assert(fan_out > 0);
    return random_gaussian(0.0f, sqrtf(2.0f / (float)(fan_in + fan_out)));
}
