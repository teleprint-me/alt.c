/**
 * @file examples/models/xor.c
 *
 * @brief Train and test the multi-layer perceptron model using XOR gates.
 *
 * @ref 1989 Multilayer Feedforward Networks are Universal Approximators
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Define the structure of the network
#define INPUT_SIZE 2 // Number of input features (e.g., XOR has 2 inputs)
#define HIDDEN_SIZE 2 // Number of neurons in the hidden layer
#define OUTPUT_SIZE 1 // Number of output neurons (e.g., XOR has 1 output)
#define NUM_SAMPLES 4 // Number of training samples (XOR gate has 4 inputs)

// Training parameters
#define LEARNING_RATE 0.1f // Learning rate for gradient descent
#define EPOCHS 10000 // Maximum number of training epochs
#define ERROR_THRESHOLD 0.01f // Early stopping threshold for average error

// Sigmoid activation function
float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Derivative of sigmoid for backpropagation
float sigmoid_derivative(float x) {
    return x * (1.0f - x);
}

// SiLU activation function
float silu(float x) {
    return x * sigmoid(x);
}

// Derivative of SiLU for backpropagation
float silu_derivative(float x) {
    float sigmoid_x = sigmoid(x);
    return sigmoid_x * (1.0f + x * (1.0f - sigmoid_x));
}

// Relu breaks the model
float relu(float x) {
    return x > 0 ? x : 0;
}

float relu_derivative(float x) {
    return x > 0 ? 1 : 0;
}

// Linear initialization for weights [0, 1]
float linear_random(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Gaussian initialization for weights
float gaussian_random(void) {
    return linear_random() * sqrtf(2.0f / INPUT_SIZE);
}

// Initialize weights and biases
void initialize_weights(
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE],
    float hidden_weights_output[HIDDEN_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float* output_bias,
    float (*random)(void)
) {
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        hidden_biases[i] = random();
        hidden_weights_output[i] = random();
        for (int j = 0; j < INPUT_SIZE; j++) {
            hidden_weights_input[j][i] = random();
        }
    }
    *output_bias = random();
}

// Forward pass
void forward(
    float input[INPUT_SIZE],
    float hidden[HIDDEN_SIZE],
    float* output,
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE],
    float hidden_weights_output[HIDDEN_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float* output_bias
) {
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        hidden[i] = hidden_biases[i];
        for (int j = 0; j < INPUT_SIZE; j++) {
            hidden[i] += hidden_weights_input[j][i] * input[j];
        }
        hidden[i] = sigmoid(hidden[i]);
    }

    *output = *output_bias;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        *output += hidden_weights_output[i] * hidden[i];
    }
    *output = sigmoid(*output);
}

void backward(
    float input[HIDDEN_SIZE],
    float hidden[HIDDEN_SIZE],
    float output,
    float target,
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE],
    float hidden_weights_output[HIDDEN_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float* output_bias
) {
    float output_error = target - output; // Error at output layer
    float output_gradient = output_error * sigmoid_derivative(output);

    // Update output weights and bias
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        hidden_weights_output[i] += LEARNING_RATE * output_gradient * hidden[i];
    }
    *output_bias += LEARNING_RATE * output_gradient;

    // Hidden layer gradients
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        float hidden_error = output_gradient * hidden_weights_output[i];
        float hidden_gradient = hidden_error * sigmoid_derivative(hidden[i]);

        // Update hidden weights and biases
        for (int j = 0; j < INPUT_SIZE; j++) {
            hidden_weights_input[j][i] += LEARNING_RATE * hidden_gradient * input[j];
        }
        hidden_biases[i] += LEARNING_RATE * hidden_gradient;
    }
}

void train(
    float inputs[NUM_SAMPLES][INPUT_SIZE],
    float targets[NUM_SAMPLES],
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE],
    float hidden_weights_output[HIDDEN_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float* output_bias,
    int epochs
) {
    for (int epoch = 0; epoch < epochs; epoch++) {
        float total_error = 0.0;
        for (int i = 0; i < NUM_SAMPLES; i++) { // Iterate over all training examples
            float hidden[HIDDEN_SIZE];
            float output;

            forward(
                inputs[i],
                hidden,
                &output,
                hidden_weights_input,
                hidden_weights_output,
                hidden_biases,
                output_bias
            );
            backward(
                inputs[i],
                hidden,
                output,
                targets[i],
                hidden_weights_input,
                hidden_weights_output,
                hidden_biases,
                output_bias
            );

            // Accumulate error
            total_error += powf(targets[i] - output, 2);
        }

        // Report progress every 1000 epochs
        if (epoch % 1000 == 0) {
            printf("Epoch %d, Error: %.6f\n", epoch, (double) total_error / NUM_SAMPLES);
        }

        // Early stopping condition
        if (total_error / NUM_SAMPLES < ERROR_THRESHOLD) {
            printf(
                "Converged at epoch %d, Error: %.6f\n", epoch, (double) total_error / NUM_SAMPLES
            );
            break;
        }
    }
}

void test(
    float inputs[NUM_SAMPLES][INPUT_SIZE],
    float targets[NUM_SAMPLES],
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE],
    float hidden_weights_output[HIDDEN_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float* output_bias
) {
    printf("Testing the trained model:\n");
    for (int i = 0; i < 4; i++) {
        float hidden[HIDDEN_SIZE], output;
        forward(
            inputs[i],
            hidden,
            &output,
            hidden_weights_input,
            hidden_weights_output,
            hidden_biases,
            output_bias
        );
        printf(
            "Input: %.0f, %.0f, Predicted: %.6f, Actual: %.0f\n",
            (double) inputs[i][0],
            (double) inputs[i][1],
            (double) output,
            (double) targets[i]
        );
    }
}

int main(void) {
    // Egg: The answer to life, the universe, and everything.
    srand(42); // k = x^3 + y^3 + z^3

    // XOR dataset
    float inputs[NUM_SAMPLES][INPUT_SIZE] = {
        {0.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f}
    };
    float targets[NUM_SAMPLES] = {0.0f, 1.0f, 1.0f, 0.0f}; // Expected XOR outputs

    // Weights and biases
    float hidden_weights_input[INPUT_SIZE][HIDDEN_SIZE]; // 2 inputs -> 2 hidden neurons
    float hidden_weights_output[HIDDEN_SIZE]; // 2 hidden neurons -> 1 output neuron
    float hidden_biases[HIDDEN_SIZE]; // 2 hidden neurons
    float output_bias; // 1 output neuron

    // Initialize weights and biases
    initialize_weights(
        hidden_weights_input, hidden_weights_output, hidden_biases, &output_bias, gaussian_random
    );

    // Train the XOR model
    train(
        inputs,
        targets,
        hidden_weights_input,
        hidden_weights_output,
        hidden_biases,
        &output_bias,
        EPOCHS // Train for 10,000 epochs
    );

    // Test the XOR model
    test(inputs, targets, hidden_weights_input, hidden_weights_output, hidden_biases, &output_bias);

    return 0;
}
