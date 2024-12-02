/**
 * @file include/activation.h
 *
 * @brief Activation functions for neural networks.
 */

#ifndef ALT_ACTIVATION_H
#define ALT_ACTIVATION_H

#include "data_types.h" // For math.h, M_PI, etc.

/**
 * @brief Binary Step Activation Function.
 *
 * A simple activation function that returns 0 for x < 0 and 1 for x >= 0.
 *
 * @param x The input value.
 * @return 1 if x >= 0, otherwise 0.
 */
float activate_binary_step(float x);

/**
 * @brief Sigmoid Activation Function.
 *
 * Computes the sigmoid activation: 1 / (1 + exp(-x)).
 *
 * @param x The input value.
 * @return A value between 0 and 1.
 */
float activate_sigmoid(float x);

/**
 * @brief Derivative of the Sigmoid Function.
 *
 * Computes the derivative of the sigmoid activation function.
 * Useful for backpropagation in neural networks.
 *
 * @param x The input value.
 * @return The derivative of the sigmoid function at x.
 */
float activate_sigmoid_prime(float x);

/**
 * @brief Hyperbolic Tangent (Tanh) Activation Function.
 *
 * Computes the tanh activation: (exp(x) - exp(-x)) / (exp(x) + exp(-x)).
 *
 * @param x The input value.
 * @return A value between -1 and 1.
 */
float activate_tanh(float x);

/**
 * @brief Rectified Linear Unit (ReLU).
 *
 * Computes the ReLU activation: max(0, x).
 *
 * @param x The input value.
 * @return 0 if x < 0, otherwise x.
 */
float activate_relu(float x);

/**
 * @brief Derivative of the ReLU Function.
 *
 * Computes the derivative of the ReLU activation function.
 *
 * @param x The input value.
 * @return 0 if x <= 0, otherwise 1.
 */
float activate_relu_prime(float x);

/**
 * @brief Sigmoid-Weighted Linear Unit (SiLU) or Swish Activation.
 *
 * Computes the SiLU activation: x / (1 + exp(-x)).
 *
 * @param x The input value.
 * @return The SiLU activation at x.
 */
float activate_silu(float x);

/**
 * @brief Derivative of the SiLU Function.
 *
 * Computes the derivative of the SiLU activation function.
 *
 * @param x The input value.
 * @return The derivative of the SiLU activation at x.
 */
float activate_silu_prime(float x);

/**
 * @brief Gaussian Error Linear Unit (GELU).
 *
 * Computes the exact GELU activation:
 * x * 0.5 * (1 + erf(x / sqrt(2))).
 *
 * @param x The input value.
 * @return The exact GELU activation at x.
 */
float activate_gelu_exact(float x);

/**
 * @brief Approximate Gaussian Error Linear Unit (GELU).
 *
 * Computes an approximate GELU activation:
 * x * sigmoid(1.702 * x).
 *
 * @param x The input value.
 * @return The approximate GELU activation at x.
 */
float activate_gelu_approximation(float x);

#endif // ALT_ACTIVATION_H
