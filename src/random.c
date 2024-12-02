/**
 * @file src/random.c
 * 
 * @brief Functions for initializing model weights.
 */

#include <assert.h>
#include <stdlib.h>

// Linear initialization [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

float random_uniform(float min, float max) {
    return min + ((float) rand() / (float) RAND_MAX) * (max - min);
}

float random_range(float interval) {
    assert(interval > 0.0f);
    float normalized = (float) rand() / (float) RAND_MAX;
    return -interval + normalized * 2.0f * interval;
}

// Gaussian initialization
float random_gaussian(int hidden_size) {
    return linear_random() * sqrtf(2.0f / (float) hidden_size);
}

// @todo he initialization

// @todo xavier and glorot initialization
