/**
 * @file src/random.c
 *
 * @brief Functions for initializing model weights.
 */

#include <assert.h> // For assert
#include <stdlib.h> // For rand and RAND_MAX

#include "data_types.h" // For math.h and M_PI

// Linear initialization [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Uniform distribution
float random_uniform(float min, float max) {
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
float random_he(int fan_in) {
    return random_gaussian(0.0f, sqrtf(2.0f / (float) fan_in));
}

// Xavier and Glorot initialization
float random_glorot(int fan_in, int fan_out) {
    return random_gaussian(0.0f, sqrtf(2.0f / (float)(fan_in + fan_out)));
}
