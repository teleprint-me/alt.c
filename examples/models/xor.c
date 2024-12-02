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

// SiLU activation function
float silu(float x) {
    return x * sigmoid(x);
}

// Derivative of SiLU for backpropagation
float silu_derivative(float x) {
    float sigmoid_x = sigmoid(x);
    return sigmoid_x * (1.0f + x * (1.0f - sigmoid_x));
}

void initialize_weights(
    float hidden_weights[INPUT_SIZE][HIDDEN_SIZE],
    float output_weights[HIDDEN_SIZE][OUTPUT_SIZE],
    float hidden_biases[HIDDEN_SIZE],
    float output_biases[OUTPUT_SIZE]
) {
    // Randomize weights and biases between -1 and 1
    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < HIDDEN_SIZE; j++) {
            hidden_weights[i][j] = ((float) rand() / RAND_MAX) * 2 - 1;
        }
    }
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        for (int j = 0; j < OUTPUT_SIZE; j++) {
            output_weights[i][j] = ((float) rand() / RAND_MAX) * 2 - 1;
        }
        hidden_biases[i] = ((float) rand() / RAND_MAX) * 2 - 1;
    }
    for (int j = 0; j < OUTPUT_SIZE; j++) {
        output_biases[j] = ((float) rand() / RAND_MAX) * 2 - 1;
    }
}

// void forward(double input[], double hidden[], double* output) {
//     // Hidden layer computation
//     for (int i = 0; i < 2; i++) {
//         hidden[i] = bias_hidden[i];
//         for (int j = 0; j < 2; j++) {
//             hidden[i] += weights_hidden_input[i][j] * input[j];
//         }
//         hidden[i] = sigmoid(hidden[i]); // Apply activation
//     }

//     // Output layer computation
//     *output = bias_output;
//     for (int i = 0; i < 2; i++) {
//         *output += weights_hidden_output[i] * hidden[i];
//     }
//     *output = silu(*output); // Apply activation
// }

// void backward(double input[], double hidden[], double output, double target) {
//     double output_error = target - output; // Error at output layer
//     double output_delta = output_error * sigmoid_derivative(output);

//     double hidden_error[2], hidden_delta[2];
//     for (int i = 0; i < 2; i++) {
//         hidden_error[i] = weights_hidden_output[i] * output_delta;
//         hidden_delta[i] = hidden_error[i] * sigmoid_derivative(hidden[i]);
//     }

//     // Update weights for hidden -> output
//     for (int i = 0; i < 2; i++) {
//         weights_hidden_output[i] += hidden[i] * output_delta * 0.1; // 0.1 = learning rate
//     }
//     bias_output += output_delta * 0.1;

//     // Update weights for input -> hidden
//     for (int i = 0; i < 2; i++) {
//         for (int j = 0; j < 2; j++) {
//             weights_hidden_input[i][j] += input[j] * hidden_delta[i] * 0.1;
//         }
//         bias_hidden[i] += hidden_delta[i] * 0.1;
//     }
// }

// void train(int epochs) {
//     for (int epoch = 0; epoch < epochs; epoch++) {
//         double total_error = 0.0;
//         for (int i = 0; i < 4; i++) { // Iterate over all training examples
//             double hidden[2], output;
//             forward(inputs[i], hidden, &output);
//             backward(inputs[i], hidden, output, outputs[i]);

//             total_error += pow(outputs[i] - output, 2); // Track error
//         }
//         if (epoch % 1000 == 0) {
//             printf("Epoch %d, Error: %f\n", epoch, total_error);
//         }
//     }
// }

// void test() {
//     printf("Testing the trained model:\n");
//     for (int i = 0; i < 4; i++) {
//         double hidden[2], output;
//         forward(inputs[i], hidden, &output);
//         printf(
//             "Input: %f, %f, Predicted: %f, Actual: %f\n",
//             inputs[i][0],
//             inputs[i][1],
//             output,
//             outputs[i]
//         );
//     }
// }

int main(void) {
    // Egg: The answer to life, the universe, and everything.
    srand(42); // Seed for reproducibility

    // // Dataset: Inputs and corresponding targets
    // float inputs[NUM_SAMPLES][INPUT_SIZE] = {
    //     {0.0f, 0.0f},
    //     {0.0f, 1.0f},
    //     {1.0f, 0.0f},
    //     {1.0f, 1.0f}
    // };
    // float targets[NUM_SAMPLES] = {0.0f, 1.0f, 1.0f, 0.0f}; // XOR gate

    // Declare weights and biases
    float hidden_weights[INPUT_SIZE][HIDDEN_SIZE];
    float output_weights[HIDDEN_SIZE][OUTPUT_SIZE];
    float hidden_biases[HIDDEN_SIZE];
    float output_biases[OUTPUT_SIZE];

    // Initialize weights and biases
    initialize_weights(hidden_weights, output_weights, hidden_biases, output_biases);

    // Debug print for weights and biases
    printf("Weights and Biases Initialized:\n");
    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < HIDDEN_SIZE; j++) {
            printf("Hidden[%d][%d]: %.2f ", i, j, (double) hidden_weights[i][j]);
        }
        printf("\n");
    }
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        printf("Hidden Bias[%d]: %.2f\n", i, (double) hidden_biases[i]);
    }

    // train(10000); // Train for 10,000 epochs
    // test();
    return 0;
}
