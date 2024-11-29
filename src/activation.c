/**
 * @file src/activation.c
 *
 * @brief Activation functions for neural networks.
 */

#include "activation.h"

// Binary Step Activation Function
float activate_binary_step(float x) {
    return x >= 0.0f ? 1.0f : 0.0f;
}

// Sigmoid Activation Function
float activate_sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Hyperbolic Tangent (Tanh) Activation Function
float activate_tanh(float x) {
    return tanhf(x);
}

// Rectified Linear Unit (ReLU)
float activate_relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

// Sigmoid-Weighted Linear Unit (SiLU) or Swish
float activate_silu(float x) {
    return x * activate_sigmoid(x);
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
