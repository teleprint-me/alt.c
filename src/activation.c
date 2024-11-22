/**
 * @file src/activation.c
 *
 * @brief Activation functions for neural networks.
 */

#include <math.h>

#include "activation.h"

// Binary Step Activation Function
double activate_binary_step(double x) {
    return x >= 0.0 ? 1.0 : 0.0;
}

// Sigmoid Activation Function
double activate_sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

// Hyperbolic Tangent (Tanh) Activation Function
double activate_tanh(double x) {
    return tanh(x);
}

// Rectified Linear Unit (ReLU)
double activate_relu(double x) {
    return x > 0.0 ? x : 0.0;
}

// Sigmoid-Weighted Linear Unit (SiLU) or Swish
double activate_silu(double x) {
    return x / (1.0 + exp(-x));
}

// Gaussian Error Linear Unit (GELU)
double activate_gelu(double x) {
    const double sqrt_pi = sqrt(M_PI);
    return 0.5 * x * (1.0 + erf(x / sqrt(2.0)));
}
