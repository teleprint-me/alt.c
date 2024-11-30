/**
 * @file include/activation.h
 *
 * @brief Activation functions for neural networks.
 */

#ifndef ALT_ACTIVATION_H
#define ALT_ACTIVATION_H

#include <math.h>
#include <stdbool.h>

#include "data_types.h"

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
