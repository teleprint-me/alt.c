/**
 * @file examples/models/perceptron.c
 * 
 * @ref 1957 The Perceptron: A Percieving and Recognizing Automaton
 */

#include <stdio.h>
#include <stdlib.h>

#define LEARNING_RATE 0.1f
#define EPOCHS 1000

// Activation function: Step function
int step_function(float sum) {
    return (sum >= 0) ? 1 : 0;
}

// Perceptron training function
void train_perceptron(float inputs[][2], int labels[], float weights[], int n_samples) {
    for (int epoch = 0; epoch < EPOCHS; ++epoch) {
        for (int i = 0; i < n_samples; ++i) {
            // Calculate weighted sum
            float weighted_sum = 0;
            for (int j = 0; j < 2; ++j) {
                weighted_sum += inputs[i][j] * weights[j];
            }

            // Predict
            int prediction = step_function(weighted_sum);

            // Update weights based on error
            int error = labels[i] - prediction;
            for (int j = 0; j < 2; ++j) {
                weights[j] += LEARNING_RATE * error * inputs[i][j];
            }
        }
    }
}

// Main function
int main() {
    // Dataset: Inputs and corresponding labels
    float inputs[4][2] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    int labels[4] = {0, 0, 0, 1}; // AND gate

    // Initialize weights
    float weights[2] = {0, 0};

    // Train perceptron
    train_perceptron(inputs, labels, weights, 4);

    // Test the perceptron
    printf("Trained Weights: %.2f, %.2f\n", (double) weights[0], (double) weights[1]);
    for (int i = 0; i < 4; ++i) {
        float weighted_sum = inputs[i][0] * weights[0] + inputs[i][1] * weights[1];
        int prediction = step_function(weighted_sum);
        printf("Input: %.0f, %.0f -> Prediction: %d\n", (double) inputs[i][0], (double) inputs[i][1], prediction);
    }

    return 0;
}
