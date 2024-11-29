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
    #define M_PI 3.14159265358979323846f
#endif

#ifndef PI
    #define PI 3.141592653589793f
#endif

#ifndef SQRT_2_PI
    #define SQRT_2_PI 0.7978845608028654f // sqrt(2 / pi)
#endif

#ifndef SQRT_2
    #define SQRT_2 1.4142135623730951f // sqrt(2)
#endif

// Binary Step Activation Function
float activate_binary_step(float x);

// Sigmoid Activation Function
float activate_sigmoid(float x);

// Hyperbolic Tangent (Tanh) Activation Function
float activate_tanh(float x);

// Rectified Linear Unit (ReLU)
float activate_relu(float x);

// Sigmoid-Weighted Linear Unit (SiLU) or Swish
float activate_silu(float x);

// Gaussian Error Linear Unit (GELU)
float activate_gelu_exact(float x);

// Approximate Gaussian Error Linear Unit (GELU)
float activate_gelu_approximation(float x);

#endif // ALT_ACTIVATION_H
