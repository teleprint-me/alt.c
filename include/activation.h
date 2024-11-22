/**
 * @file include/activation.h
 *
 * @brief Activation functions for neural networks.
 */

#ifndef ALT_ACTIVATION_H
#define ALT_ACTIVATION_H

#include <math.h>
#include <stdbool.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef PI
    #define PI 3.141592653589793
#endif

#ifndef SQRT_2_PI
    #define SQRT_2_PI 0.7978845608028654 // sqrt(2 / pi)
#endif

#ifndef SQRT_2
    #define SQRT_2 1.4142135623730951 // sqrt(2)
#endif

// Binary Step Activation Function
double activate_binary_step(double x);

// Sigmoid Activation Function
double activate_sigmoid(double x);

// Hyperbolic Tangent (Tanh) Activation Function
double activate_tanh(double x);

// Rectified Linear Unit (ReLU)
double activate_relu(double x);

// Sigmoid-Weighted Linear Unit (SiLU) or Swish
double activate_silu(double x);

// Gaussian Error Linear Unit (GELU)
double activate_gelu(double x, bool approximate);

#endif // ALT_ACTIVATION_H
