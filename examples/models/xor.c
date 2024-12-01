/**
 * @file examples/mlp.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Sigmoid activation function and its derivative
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}

// XOR dataset
double inputs[4][2] = {
    {0, 0},
    {0, 1},
    {1, 0},
    {1, 1}
};

double outputs[4] = {0, 1, 1, 0}; // Expected XOR outputs

// Weights and biases
double weights_input_hidden[2][2]; // 2 inputs -> 2 hidden neurons
double weights_hidden_output[2];  // 2 hidden neurons -> 1 output neuron
double bias_hidden[2];
double bias_output;

void initialize_weights() {
    for (int i = 0; i < 2; i++) {
        bias_hidden[i] = (double)rand() / RAND_MAX;
        weights_hidden_output[i] = (double)rand() / RAND_MAX;
        for (int j = 0; j < 2; j++) {
            weights_input_hidden[i][j] = (double)rand() / RAND_MAX;
        }
    }
    bias_output = (double)rand() / RAND_MAX;
}

void forward(double input[], double hidden[], double *output) {
    // Hidden layer computation
    for (int i = 0; i < 2; i++) {
        hidden[i] = bias_hidden[i];
        for (int j = 0; j < 2; j++) {
            hidden[i] += weights_input_hidden[i][j] * input[j];
        }
        hidden[i] = sigmoid(hidden[i]); // Apply activation
    }

    // Output layer computation
    *output = bias_output;
    for (int i = 0; i < 2; i++) {
        *output += weights_hidden_output[i] * hidden[i];
    }
    *output = sigmoid(*output); // Apply activation
}

void backward(double input[], double hidden[], double output, double target) {
    double output_error = target - output; // Error at output layer
    double output_delta = output_error * sigmoid_derivative(output);

    double hidden_error[2], hidden_delta[2];
    for (int i = 0; i < 2; i++) {
        hidden_error[i] = weights_hidden_output[i] * output_delta;
        hidden_delta[i] = hidden_error[i] * sigmoid_derivative(hidden[i]);
    }

    // Update weights for hidden -> output
    for (int i = 0; i < 2; i++) {
        weights_hidden_output[i] += hidden[i] * output_delta * 0.1; // 0.1 = learning rate
    }
    bias_output += output_delta * 0.1;

    // Update weights for input -> hidden
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            weights_input_hidden[i][j] += input[j] * hidden_delta[i] * 0.1;
        }
        bias_hidden[i] += hidden_delta[i] * 0.1;
    }
}

void train(int epochs) {
    for (int epoch = 0; epoch < epochs; epoch++) {
        double total_error = 0.0;
        for (int i = 0; i < 4; i++) { // Iterate over all training examples
            double hidden[2], output;
            forward(inputs[i], hidden, &output);
            backward(inputs[i], hidden, output, outputs[i]);

            total_error += pow(outputs[i] - output, 2); // Track error
        }
        if (epoch % 1000 == 0) {
            printf("Epoch %d, Error: %f\n", epoch, total_error);
        }
    }
}

void test() {
    printf("Testing the trained model:\n");
    for (int i = 0; i < 4; i++) {
        double hidden[2], output;
        forward(inputs[i], hidden, &output);
        printf("Input: %f, %f, Predicted: %f, Actual: %f\n",
               inputs[i][0], inputs[i][1], output, outputs[i]);
    }
}

int main() {
    srand(42); // Seed for reproducibility
    initialize_weights();
    train(10000); // Train for 10,000 epochs
    test();
    return 0;
}
