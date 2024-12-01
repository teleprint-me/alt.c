/**
 * @file examples/models/perceptron.c
 *
 * @ref 1957 The Perceptron: A Percieving and Recognizing Automaton
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Define macros for input dimensions
#define INPUTS 4 // 4x2 (n_samples, n_inputs)
#define WEIGHTS 2 // 2x1 (n_inputs, n_outputs)
#define OUTPUTS 4 // 4x1 (n_samples, n_outputs)
#define LEARNING_RATE 0.1f
#define EPOCHS 10000

// Sigmoid activation function
float sigmoid_activation(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Derivative of sigmoid for backpropagation
float sigmoid_derivative(float x) {
    return x * (1.0f - x);
}

// Calculate weighted sum
float dot_product(float inputs[][WEIGHTS], float* weights, float bias, int row) {
    float weighted_sum = bias;
    for (int col = 0; col < WEIGHTS; col++) {
        weighted_sum += inputs[row][col] * weights[col];
    }
    return weighted_sum;
}

// Calculate activation
float predict(float inputs[][WEIGHTS], float* weights, float bias, int row) {
    float weighted_sum = dot_product(inputs, weights, bias, row);
    float activation = sigmoid_activation(weighted_sum);
    return activation;
}

float error(float inputs[][WEIGHTS], float* targets, float* weights, float prediction, int row) {
    // Compute error
    float residual = targets[row] - prediction;

    // Update weights and bias
    for (int col = 0; col < WEIGHTS; ++col) {
        weights[col] += LEARNING_RATE * residual * sigmoid_derivative(prediction) * inputs[row][col];
    }
    return LEARNING_RATE * residual * sigmoid_derivative(prediction);
}

// Perceptron training function
void train_perceptron(float inputs[][WEIGHTS], float* targets, float* weights, float* bias) {
    for (int epoch = 0; epoch < EPOCHS; ++epoch) {
        for (int row = 0; row < INPUTS; ++row) {
            // Apply activation function
            float prediction = predict(inputs, weights, *bias, row);
            *bias += error(inputs, targets, weights, prediction, row);
        }
    }
}

// Main function
int main() {
    // Dataset: Inputs and corresponding targets
    float inputs[INPUTS][WEIGHTS] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    float targets[OUTPUTS] = {0, 0, 0, 1}; // AND gate

    // Initialize weights and bias
    float weights[WEIGHTS];
    float bias = ((float) rand() / RAND_MAX) * 2 - 1; // Random value in [-1, 1]
    for (int i = 0; i < WEIGHTS; i++) {
        weights[i] = ((float) rand() / RAND_MAX) * 2 - 1; // Random values in [-1, 1]
    }

    // Train perceptron
    train_perceptron(inputs, targets, weights, &bias);

    // Test the perceptron
    printf(
        "Trained Weights: %.2f, %.2f | Bias: %.2f\n",
        (double) weights[0],
        (double) weights[1],
        (double) bias
    );
    for (int i = 0; i < INPUTS; ++i) {
        float weighted_sum = bias;
        for (int j = 0; j < WEIGHTS; ++j) {
            weighted_sum += inputs[i][j] * weights[j];
        }
        float prediction = sigmoid_activation(weighted_sum) >= 0.5f ? 1.0f : 0.0f;
        printf(
            "Input: %.0f, %.0f -> Prediction: %.0f\n",
            (double) inputs[i][0],
            (double) inputs[i][1],
            (double) prediction
        );
    }

    return 0;
}
