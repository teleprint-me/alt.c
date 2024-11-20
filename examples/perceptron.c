/**
 * @file examples/perceptron.c
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tensors.h"

// Define macros for input dimensions
#define INPUT_ROWS 4
#define INPUT_COLS 2
#define INPUT_RANK 2

#define WEIGHTS_ROWS INPUT_COLS
#define WEIGHTS_COLS 1
#define WEIGHTS_RANK 2

#define OUTPUT_ROWS INPUT_ROWS
#define OUTPUT_COLS 1
#define OUTPUT_RANK 2

// Default hyperparameter values
#define DEFAULT_LEARNING_RATE 1e-5f
#define DEFAULT_N_EPOCHS 10
#define DEFAULT_N_SAMPLES 4
#define DEFAULT_N_INPUTS 2
#define DEFAULT_N_OUTPUTS 1

// Hyperparameters structure
typedef struct Parameters {
    float learning_rate; // Learning rate for weight updates
    unsigned int n_epochs; // Number of training epochs
    unsigned int n_samples; // Number of training samples
    unsigned int n_inputs; // Number of input features
    unsigned int n_outputs; // Number of output nodes
} Parameters;

// Create Parameters
Parameters* create_parameters(
    unsigned int n_inputs,
    unsigned int n_outputs,
    unsigned int n_epochs,
    unsigned int n_samples,
    float learning_rate
) {
    Parameters* params = (Parameters*) malloc(sizeof(Parameters));
    if (!params) {
        fprintf(stderr, "Error: Memory allocation failed for Parameters.\n");
        exit(EXIT_FAILURE);
    }

    params->n_inputs = n_inputs ? n_inputs : DEFAULT_N_INPUTS;
    params->n_outputs = n_outputs ? n_outputs : DEFAULT_N_OUTPUTS;
    params->n_epochs = n_epochs ? n_epochs : DEFAULT_N_EPOCHS;
    params->n_samples = n_samples ? n_samples : DEFAULT_N_SAMPLES;
    params->learning_rate = learning_rate > 0.0f ? learning_rate : DEFAULT_LEARNING_RATE;

    return params;
}

// Free Parameters
void free_parameters(Parameters* params) {
    if (params) {
        free(params);
    }
}

// Perceptron structure
typedef struct Perceptron {
    float bias;
    Tensor* x; // inputs shape is 4x2 (n_samples, n_inputs)
    Tensor* w; // weights shape is 2x1 (x_i * w + bias)
    Tensor* o; // predictions shape is 4x1 (n_samples, 1)
    Tensor* t; // targets shape matches predictions shape
    Parameters* params;
} Perceptron;

// Initialize perceptron
Perceptron* create_perceptron(Parameters* params, float bias) {
    Perceptron* perceptron = (Perceptron*) malloc(sizeof(Perceptron));
    if (!perceptron) {
        fprintf(stderr, "Error: Memory allocation failed for Perceptron.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int input_shape[INPUT_RANK] = {params->n_samples, params->n_inputs};
    unsigned int weight_shape[WEIGHTS_RANK] = {params->n_inputs, 1};
    unsigned int output_shape[OUTPUT_RANK] = {params->n_samples, params->n_outputs};

    // Tensors are zero-initialized upon creation
    perceptron->x = tensor_create(input_shape, INPUT_RANK); // Inputs
    perceptron->w = tensor_create(weight_shape, WEIGHTS_RANK); // Weights
    perceptron->o = tensor_create(output_shape, OUTPUT_RANK); // Outputs
    perceptron->t = tensor_create(output_shape, OUTPUT_RANK); // Targets

    perceptron->params = params;
    perceptron->bias = bias;

    return perceptron;
}

// Free perceptron
void free_perceptron(Perceptron* perceptron) {
    if (perceptron) {
        // free the inputs
        if (perceptron->x) {
            tensor_free(perceptron->x);
        }
        // free the weights
        if (perceptron->w) {
            tensor_free(perceptron->w);
        }
        // free the predictions
        if (perceptron->o) {
            tensor_free(perceptron->o);
        }
        // free the targets (ground truth)
        if (perceptron->t) {
            tensor_free(perceptron->t);
        }
        free(perceptron);
    }
}

// Populate inputs for AND truth table
void initialize_inputs(Tensor* inputs) {
    float input_data[INPUT_ROWS][INPUT_COLS] = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };

    for (unsigned int i = 0; i < INPUT_ROWS; i++) {
        for (unsigned int j = 0; j < INPUT_COLS; j++) {
            tensor_set_element(inputs, (unsigned int[]){i, j}, input_data[i][j]);
        }
    }
}

void initialize_targets(Tensor* targets) {
    float output_data[OUTPUT_ROWS] = {0.0, 0.0, 0.0, 1.0}; // AND truth table results
    for (unsigned int i = 0; i < OUTPUT_ROWS; i++) {
        tensor_set_element(targets, (unsigned int[]){i, 0}, output_data[i]);
    }
}

// @note Can probably use xavier or he initialization.
// doesn't matter right now. something to think about.

// Initialize weights randomly in the range [0, 1]
void initialize_weights(Tensor* weights) {
    for (unsigned int i = 0; i < weights->shape[0]; i++) {
        unsigned int index[WEIGHTS_RANK] = {i, 0};
        float random_weight = (float) rand() / RAND_MAX; // Range [0, 1]
        tensor_set_element(weights, index, random_weight);
    }
}

// Alternative: Initialize weights in the range [-1, 1]
void initialize_weights_alternative(Tensor* weights) {
    for (unsigned int i = 0; i < weights->shape[0]; i++) {
        unsigned int index[WEIGHTS_RANK] = {i, 0};
        float random_weight = ((float) rand() / RAND_MAX) * 2 - 1; // Range [-1, 1]
        tensor_set_element(weights, index, random_weight);
    }
}

// @note can probably use sigmoid, silu, relu, gelu, etc.
// doesn't matter right now. something to think about.

// Activation function
float binary_step_activation(float x) {
    return (x >= 0.0f) ? 1.0f : 0.0f;
}

float sigmoid_activation(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Compute dot product using tensors
float calculate_row_dot_product(Tensor* x, Tensor* w, float bias, unsigned int row) {
    float dot_product = 0.0f;
    for (unsigned int i = 0; i < x->shape[1]; i++) {
        dot_product += tensor_get_element(x, (unsigned int[]){row, i})
                       * tensor_get_element(w, (unsigned int[]){i, 0});
    }
    return dot_product + bias;
}

// Feed-forward using the perceptron
float predict(Perceptron* p, unsigned int row, float (*activation_fn)(float)) {
    float sum = calculate_row_dot_product(p->x, p->w, p->bias, row);
    return activation_fn(sum);
}

// Back-propagation (error correction)
void update_weights(Perceptron* p, unsigned int row, float error) {
    for (unsigned int j = 0; j < p->params->n_inputs; j++) {
        float weight = tensor_get_element(p->w, (unsigned int[]){j, 0});
        float input = tensor_get_element(p->x, (unsigned int[]){row, j});
        weight += p->params->learning_rate * error * input;
        tensor_set_element(p->w, (unsigned int[]){j, 0}, weight);
    }
    p->bias += p->params->learning_rate * error;
}

// Train perceptron using the perceptron learning rule
void train(Perceptron* p, float (*activation_fn)(float)) {
    for (unsigned int epoch = 0; epoch < p->params->n_epochs; epoch++) {
        printf("Epoch %u:\n", epoch + 1);

        for (unsigned int i = 0; i < p->x->shape[0]; i++) { // Iterate over rows (samples)
            // Predict the output for the current row
            float predicted = predict(p, i, activation_fn);
            // Set the models predicted output
            tensor_set_element(p->o, (unsigned int[]){i, 0}, predicted);

            // Calculate the error using the ground truth
            float actual = tensor_get_element(p->t, (unsigned int[]){i, 0});
            float error = actual - predicted;

            // Update weights and bias
            update_weights(p, i, error);

            // Debugging: Print weights and bias
            printf("  Weighted Sum (Row %u): %.2f\n", i, (double) predicted);
            printf("  Sample %u: Error = %.2f, Weights = [", i, (double) error);
            for (unsigned int j = 0; j < p->w->shape[0]; j++) {
                printf("%.2f", (double) tensor_get_element(p->w, (unsigned int[]){j, 0}));
                if (j < p->w->shape[0] - 1) {
                    printf(", ");
                }
            }
            printf("], Bias = %.2f\n", (double) p->bias);
        }
        printf("\n");
    }
}

// Main function
int main() {
    // Seed random number generator for reproducibility
    srand((unsigned int) time(NULL));

    // Create perceptron parameters
    Parameters* params = create_parameters(
        /* n_inputs */ 2,
        /* n_outputs */ 1,
        /* n_epochs */ 10,
        /* n_samples */ 4,
        /* learning rate */ 0.1f
    );

    // Create and initialize the perceptron
    Perceptron* perceptron = create_perceptron(params, /* bias */ 0.0f);

    // Initialize inputs and expected outputs (AND truth table)
    initialize_inputs(perceptron->x);

    // Initialize weights
    initialize_weights(perceptron->w);

    // Initialize ground truth
    initialize_targets(perceptron->t);

    printf("Initialized Weights = [");
    for (unsigned int j = 0; j < perceptron->w->shape[0]; j++) {
        printf("%.2f", (double) tensor_get_element(perceptron->w, (unsigned int[]){j, 0}));
        if (j < perceptron->w->shape[0] - 1) {
            printf(", ");
        }
    }
    printf("], Bias = %.2f\n", (double) perceptron->bias);

    // Train the perceptron using the binary step activation function
    train(perceptron, binary_step_activation);

    // Test the trained perceptron
    printf("Testing trained perceptron:\n");
    for (unsigned int i = 0; i < perceptron->x->shape[0]; i++) {
        // Predict output for each row
        float predicted = predict(perceptron, i, binary_step_activation);
        float actual = tensor_get_element(perceptron->t, (unsigned int[]){i, 0});
        printf(
            "  Input: [%.1f, %.1f], Predicted: %.1f, Actual: %.1f\n",
            (double) tensor_get_element(perceptron->x, (unsigned int[]){i, 0}),
            (double) tensor_get_element(perceptron->x, (unsigned int[]){i, 1}),
            (double) predicted,
            (double) actual
        );
    }

    // Clean up memory
    free_parameters(params);
    free_perceptron(perceptron);

    return 0;
}
