/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/activation.c
 *
 * @brief Activation functions for neural networks.
 *
 * Includes common activation functions and their derivatives for backpropagation.
 * Covers basic functions (e.g., Sigmoid, ReLU) and advanced ones (e.g., GELU, SiLU).
 */

#include "interface/activation.h"

// Binary Step Activation Function
float activate_binary_step(float x) {
    return x >= 0.0f ? 1.0f : 0.0f;
}

// Sigmoid Activation Function
float activate_sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Derivative of sigmoid for backpropagation
float activate_sigmoid_prime(float x) {
    return x * (1.0f - x);
}

// Hyperbolic Tangent (Tanh) Activation Function
float activate_tanh(float x) {
    return tanhf(x);
}

// Rectified Linear Unit (ReLU)
float activate_relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

// Derivative of relu for backpropagation
float activate_relu_prime(float x) {
    return x > 0.0f ? 1.0f : 0.0f;
}

// Sigmoid-Weighted Linear Unit (SiLU) or Swish
float activate_silu(float x) {
    return x * activate_sigmoid(x);
}

// Derivative of SiLU for backpropagation
float activate_silu_prime(float x) {
    float sigmoid_x = activate_sigmoid(x);
    return sigmoid_x * (1.0f + x * (1.0f - sigmoid_x));
}

// Gaussian Error Linear Unit (GELU)
float activate_gelu_exact(float x) {
    // Exact computation using Gaussian CDF
    return 0.5f * x * (1.0f + erff(x / SQRT_2));
}

// Approximate Gaussian Error Linear Unit (GELU)
float activate_gelu_approximation(float x) {
    // Tanh-based approximation
    float x_cubed = x * x * x;
    return 0.5f * x * (1.0f + tanhf(SQRT_2_PI * (x + 0.044715f * x_cubed)));
}

// Computes the softmax function for a 1D array.
void activate_softmax(const float *input, float *output, size_t length) {
    assert(input != NULL && output != NULL);
    assert(length > 0);

    // Step 1: Find the maximum value for numerical stability
    float max_val = input[0];
    for (size_t i = 1; i < length; i++) {
        if (input[i] > max_val) {
            max_val = input[i];
        }
    }

    // Step 2: Compute the exponentials and their sum
    float sum = 0.0f;
    for (size_t i = 0; i < length; i++) {
        output[i] = expf(input[i] - max_val);
        sum += output[i];
    }

    // Step 3: Normalize the exponentials
    for (size_t i = 0; i < length; i++) {
        output[i] /= sum;
    }
}
