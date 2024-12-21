/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/random.h
 *
 * @brief Functions for generating random numbers and initializing model weights.
 *
 * Provides utility functions for generating random numbers in uniform and Gaussian distributions.
 * Includes specific weight initialization strategies such as He and Glorot initialization,
 * commonly used in machine learning applications.
 */

#ifndef ALT_RANDOM_H
#define ALT_RANDOM_H

#include <stdlib.h> // For rand and RAND_MAX

#include "data_types.h" // For assert.h, math.h, and M_PI

/**
 * @brief Seeds the random number generator.
 *
 * Seeds the random number generator using the provided seed value.
 * Should be called before using other random functions to ensure reproducibility.
 *
 * @param seed The seed value to initialize the random number generator.
 */
void random_seed(uint32_t seed);

/**
 * @brief Generates a random number in the range [0, 1].
 *
 * Uses the standard library's `rand()` function to generate a normalized random
 * float in the range [0, 1].
 *
 * @return A random float between 0.0 and 1.0.
 */
float random_linear(void);

/**
 * @brief Initializes a flat vector using random_linear.
 *
 * This function initializes the given array with random numbers in the range [0, 1].
 * The array's size should be provided as width for vectors.
 *
 * This function is useful for initializing model weights or biases in neural networks.
 *
 * @param vector The array to be initialized.
 * @param width The number of columns in the vector.
 */
void random_linear_init_vector(float* vector, uint32_t width);

/**
 * @brief Initializes a flat matrix using random_linear.
 *
 * This function initializes the given array with random numbers in the range [0, 1].
 *
 * This function is useful for initializing model weights or biases in neural networks.
 * If the array represents a matrix, the height and width parameters should be provided.
 *
 * @param matrix The array to be initialized.
 * @param width The number of columns in the matrix.
 * @param height The number of rows in the matrix.
 */
void random_linear_init_matrix(float* matrix, uint32_t width, uint32_t height);

/**
 * @brief Generates a random number in a specified uniform range.
 *
 * Produces a random number within the range [min, max), where `min` is inclusive
 * and `max` is exclusive.
 *
 * @param min The minimum value (inclusive).
 * @param max The maximum value (exclusive).
 * @return A random float in the range [min, max).
 * @note Asserts that `max > min`.
 */
float random_uniform(float min, float max);

/**
 * @brief Generates a random number following a Gaussian (normal) distribution.
 *
 * Implements the Box–Muller transform to produce a normally distributed random
 * number with a specified mean and standard deviation.
 *
 * @param mean The mean (μ) of the Gaussian distribution.
 * @param stddev The standard deviation (σ) of the Gaussian distribution.
 * @return A random float sampled from the specified Gaussian distribution.
 */
float random_gaussian(float mean, float stddev);

/**
 * @brief Initializes weights using He initialization.
 *
 * Generates weights suitable for layers with ReLU activations by sampling from a
 * Gaussian distribution scaled to the number of input units (fan-in).
 *
 * @param fan_in The number of input units to the layer.
 * @return A random float initialized according to He initialization.
 * @note Asserts that `fan_in > 0`.
 */
float random_kaiming_he(int32_t fan_in);

/**
 * @brief Initializes weights using Xavier (Glorot) initialization.
 *
 * Generates weights suitable for layers with sigmoid or tanh activations by sampling
 * from a Gaussian distribution scaled to the number of input and output units.
 *
 * @param fan_in The number of input units to the layer.
 * @param fan_out The number of output units from the layer.
 * @return A random float initialized according to Xavier (Glorot) initialization.
 * @note Asserts that both `fan_in > 0` and `fan_out > 0`.
 */
float random_xavier_glorot(int32_t fan_in, int32_t fan_out);

#endif // ALT_RANDOM_H
