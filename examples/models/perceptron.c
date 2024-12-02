/**
 * @file examples/models/perceptron.c
 *
 * @brief Train and test the perceptron model using AND gates.
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
#define ERROR_THRESHOLD 0.01f // Early stopping threshold

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

// The forward pass
float predict(float inputs[][WEIGHTS], float* weights, float bias, int row) {
    float weighted_sum = dot_product(inputs, weights, bias, row);
    return sigmoid_activation(weighted_sum);
}

// Compute error (residual)
float compute_error(float* targets, float prediction, int row) {
    return targets[row] - prediction;
}

// Update the bias
float update_bias(float residual, float prediction) {
    return LEARNING_RATE * residual * sigmoid_derivative(prediction);
}

// Update the weights
void update_weights(
    float inputs[][WEIGHTS], float* weights, float residual, float prediction, int row
) {
    for (int col = 0; col < WEIGHTS; ++col) {
        weights[col]
            += LEARNING_RATE * residual * sigmoid_derivative(prediction) * inputs[row][col];
    }
}

// Perceptron training function
void train_perceptron(float inputs[][WEIGHTS], float* targets, float* weights, float* bias) {
    for (int epoch = 0; epoch < EPOCHS; ++epoch) {
        float total_error = 0.0f;

        for (int row = 0; row < INPUTS; ++row) {
            // Forward pass
            float prediction = predict(inputs, weights, *bias, row);

            // Compute residual and update weights/bias
            float residual = compute_error(targets, prediction, row);
            update_weights(inputs, weights, residual, prediction, row);
            *bias += update_bias(residual, prediction);

            // Accumulate error for early stopping
            total_error += fabsf(residual);
        }

        // Report progress
        if (epoch % 1000 == 0) {
            printf("Epoch %d: Average Error: %.5f\n", epoch, (double) (total_error / INPUTS));
        }

        // Early stopping
        if ((total_error / INPUTS) < ERROR_THRESHOLD) {
            printf("Converged at epoch %d with average error %.5f\n", epoch, (double) (total_error / INPUTS));
            break;
        }
    }
}

// Test the perceptron
void test_perceptron(float inputs[][WEIGHTS], float* weights, float bias) {
    printf(
        "Trained Weights: %.2f, %.2f | Bias: %.2f\n",
        (double) weights[0],
        (double) weights[1],
        (double) bias
    );
    for (int row = 0; row < INPUTS; ++row) {
        float prediction = predict(inputs, weights, bias, row) >= 0.5f ? 1.0f : 0.0f;
        printf(
            "Input: %.0f, %.0f -> Prediction: %.0f\n",
            (double) inputs[row][0],
            (double) inputs[row][1],
            (double) prediction
        );
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
    test_perceptron(inputs, weights, bias);

    return 0;
}
