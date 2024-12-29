/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/perceptron.c
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "interface/logger.h"
#include "interface/data_types.h"
#include "interface/activation.h"
#include "interface/flex_array.h"
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
    uint32_t n_epochs; // Number of training epochs
    uint32_t n_samples; // Number of training samples
    uint32_t n_inputs; // Number of input features
    uint32_t n_outputs; // Number of output nodes
} Parameters;

// Create Parameters
Parameters* create_parameters(
    uint32_t n_inputs,
    uint32_t n_outputs,
    uint32_t n_epochs,
    uint32_t n_samples,
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
    Tensor* x; // input shape is 4x2 (n_samples, n_inputs)
    Tensor* w; // weight shape is 2x1 (n_inputs, n_outputs)
    Tensor* o; // output prediction shape is 4x1 (n_samples, n_outputs)
    Tensor* t; // target shape matches output shape
    Parameters* params;
} Perceptron;

// Initialize perceptron
Perceptron* create_perceptron(Parameters* params, float bias) {
    if (!params) {
        LOG_ERROR("Parameters must not be NULL.\n");
        return NULL;
    }

    Perceptron* perceptron = (Perceptron*) malloc(sizeof(Perceptron));
    if (!perceptron) {
        LOG_ERROR("Memory allocation failed for Perceptron.\n");
        return NULL;
    }

    // Create tensors
    perceptron->x = tensor_create(TYPE_FLOAT32, INPUT_RANK, (uint32_t[]){params->n_samples, params->n_inputs}); // Inputs
    perceptron->w = tensor_create(TYPE_FLOAT32, WEIGHTS_RANK, (uint32_t[]){params->n_inputs, params->n_outputs}); // Weights
    perceptron->o = tensor_create(TYPE_FLOAT32, OUTPUT_RANK, (uint32_t[]){params->n_samples, params->n_outputs}); // Outputs
    perceptron->t = tensor_create(TYPE_FLOAT32, OUTPUT_RANK, (uint32_t[]){params->n_samples, params->n_outputs}); // Targets

    if (!perceptron->x || !perceptron->w || !perceptron->o || !perceptron->t) {
        LOG_ERROR("Tensor creation failed.\n");
        free_perceptron(perceptron); // Cleanup
        return NULL;
    }

    perceptron->params = params;
    perceptron->bias = bias;

    LOG_DEBUG("Perceptron successfully created.\n");
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
TensorState initialize_inputs(Tensor* inputs) {
    if (!inputs) {
        LOG_ERROR("Invalid tensor provided to initialize_inputs.\n");
        return TENSOR_ERROR;
    }

    LOG_DEBUG("Initializing tensor inputs for AND truth table (shape [%u, %u])...\n", INPUT_ROWS, INPUT_COLS);

    // AND truth table inputs (4x2 matrix)
    float input_data[INPUT_ROWS][INPUT_COLS] = {
        {0.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f}
    };

    // Set the tensor data in bulk
    TensorState state = tensor_set_bulk(inputs, input_data);
    if (state != TENSOR_SUCCESS) {
        LOG_ERROR("Failed to set tensor inputs in bulk (TensorState: %d).\n", state);
        return state; // Propagate error state
    }

    LOG_DEBUG("Tensor inputs successfully initialized.\n");
    return TENSOR_SUCCESS;
}

TensorState initialize_targets(Tensor* targets) {
    if (!targets) {
        LOG_ERROR("Invalid tensor provided to initialize_targets.\n");
        return TENSOR_ERROR;
    }

    LOG_DEBUG("Initializing target tensor for AND truth table (shape [%u, %u])...\n",
              OUTPUT_ROWS, OUTPUT_COLS);

    // AND truth table results (4x1 column vector)
    float output_data[OUTPUT_ROWS][OUTPUT_COLS] = {
        {0.0f},
        {0.0f},
        {0.0f},
        {1.0f}
    };

    // Set the tensor data in bulk
    TensorState state = tensor_set_bulk(targets, output_data);
    if (state != TENSOR_SUCCESS) {
        LOG_ERROR("Failed to initialize target tensor in bulk (TensorState: %d).\n", state);
        return state;
    }

    LOG_DEBUG("Target tensor successfully initialized.\n");
    return TENSOR_SUCCESS;
}

// @note Can probably use xavier or he initialization.
// doesn't matter right now. something to think about.

// Initialize weights randomly in the range [0, 1]
TensorState initialize_weights(Tensor* weights) {
    if (!weights) {
        LOG_ERROR("Invalid tensor provided to initialize_weights.\n");
        return TENSOR_ERROR;
    }

    LOG_DEBUG("Initializing weights tensor with random values...\n");

    uint32_t rows, cols;
    FlexState flex_state;

    // Get the dimensions of the weights tensor
    flex_state = flex_array_get(weights->shape, 0, &rows); // Number of rows
    if (flex_state != FLEX_ARRAY_SUCCESS) {
        LOG_ERROR("Failed to get the number of rows for weights tensor.\n");
        return TENSOR_INVALID_SHAPE;
    }

    flex_state = flex_array_get(weights->shape, 1, &cols); // Number of columns
    if (flex_state != FLEX_ARRAY_SUCCESS) {
        LOG_ERROR("Failed to get the number of columns for weights tensor.\n");
        return TENSOR_INVALID_SHAPE;
    }

    // Randomly initialize weights
    float random_weights[rows][cols];
    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            random_weights[i][j] = (float) rand() / (float) RAND_MAX; // Random value in range [0, 1]
        }
    }

    // Set the weights tensor in bulk
    TensorState state = tensor_set_bulk(weights, random_weights);
    if (state != TENSOR_SUCCESS) {
        LOG_ERROR("Failed to set weights tensor in bulk.\n");
        return state;
    }

    LOG_DEBUG("Weights tensor successfully initialized.\n");
    return TENSOR_SUCCESS;
}

// Compute dot product using tensors
TensorState calculate_row_dot_product(Tensor* x, Tensor* w, float bias, uint32_t row, float* result) {
    if (!x || !w || !result) {
        LOG_ERROR("Invalid arguments provided to calculate_row_dot_product.\n");
        return TENSOR_ERROR;
    }

    uint32_t n_inputs;
    if (flex_array_get(x->shape, 1, &n_inputs) != FLEX_ARRAY_SUCCESS) {
        LOG_ERROR("Failed to retrieve input dimension for x.\n");
        return TENSOR_INVALID_SHAPE;
    }

    float dot_product = 0.0f;
    for (uint32_t i = 0; i < n_inputs; i++) {
        // Get the element from x
        FlexArray* x_indices = tensor_create_indices(2, (uint32_t[]){row, i});
        if (!x_indices) {
            LOG_ERROR("Failed to create indices for x tensor.\n");
            return TENSOR_MEMORY_ALLOCATION_FAILED;
        }

        float x_value;
        if (tensor_get_element(x, x_indices, &x_value) != TENSOR_SUCCESS) {
            LOG_ERROR("Failed to retrieve element from x tensor at row %u, col %u.\n", row, i);
            flex_array_free(x_indices);
            return TENSOR_ERROR;
        }

        // Get the element from w
        FlexArray* w_indices = tensor_create_indices(2, (uint32_t[]){i, 0});
        if (!w_indices) {
            LOG_ERROR("Failed to create indices for w tensor.\n");
            flex_array_free(x_indices);
            return TENSOR_MEMORY_ALLOCATION_FAILED;
        }

        float w_value;
        if (tensor_get_element(w, w_indices, &w_value) != TENSOR_SUCCESS) {
            LOG_ERROR("Failed to retrieve element from w tensor at row %u.\n", i);
            flex_array_free(x_indices);
            flex_array_free(w_indices);
            return TENSOR_ERROR;
        }

        // Accumulate dot product
        dot_product += x_value * w_value;

        // Clean up
        flex_array_free(x_indices);
        flex_array_free(w_indices);
    }

    *result = dot_product + bias;
    return TENSOR_SUCCESS;
}

// Feed-forward using the perceptron
float predict(Perceptron* p, uint32_t row, float (*activation_fn)(float)) {
    float sum = calculate_row_dot_product(p->x, p->w, p->bias, row);
    return activation_fn(sum);
}

// Back-propagation (error correction)
void update_weights(Perceptron* p, uint32_t row, float error) {
    for (uint32_t j = 0; j < p->params->n_inputs; j++) {
        float weight = tensor_get_element(p->w, (uint32_t[]){j, 0});
        float input = tensor_get_element(p->x, (uint32_t[]){row, j});
        weight += p->params->learning_rate * error * input;
        tensor_set_element(p->w, (uint32_t[]){j, 0}, weight);
    }
    p->bias += p->params->learning_rate * error;
}

// Train perceptron using the perceptron learning rule
void train(Perceptron* p, float (*activation_fn)(float)) {
    for (uint32_t epoch = 0; epoch < p->params->n_epochs; epoch++) {
        printf("Epoch %u:\n", epoch + 1);

        for (uint32_t i = 0; i < p->x->shape[0]; i++) { // Iterate over rows (samples)
            // Predict the output for the current row
            float predicted = predict(p, i, activation_fn);
            // Set the models predicted output
            tensor_set_element(p->o, (uint32_t[]){i, 0}, predicted);

            // Calculate the error using the ground truth
            float actual = tensor_get_element(p->t, (uint32_t[]){i, 0});
            float error = actual - predicted;

            // Update weights and bias
            update_weights(p, i, error);

            // Debugging: Print weights and bias
            printf("  Weighted Sum (Row %u): %.2f\n", i, (double) predicted);
            printf("  Sample %u: Error = %.2f, Weights = [", i, (double) error);
            for (uint32_t j = 0; j < p->w->shape[0]; j++) {
                printf("%.2f", (double) tensor_get_element(p->w, (uint32_t[]){j, 0}));
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
    srand((uint32_t) time(NULL));

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
    for (uint32_t j = 0; j < perceptron->w->shape[0]; j++) {
        printf("%.2f", (double) tensor_get_element(perceptron->w, (uint32_t[]){j, 0}));
        if (j < perceptron->w->shape[0] - 1) {
            printf(", ");
        }
    }
    printf("], Bias = %.2f\n", (double) perceptron->bias);

    // Train the perceptron using the binary step activation function
    train(perceptron, activate_binary_step);

    // Test the trained perceptron
    printf("Testing trained perceptron:\n");
    for (uint32_t i = 0; i < perceptron->x->shape[0]; i++) {
        // Predict output for each row
        float predicted = predict(perceptron, i, activate_binary_step);
        float actual = tensor_get_element(perceptron->t, (uint32_t[]){i, 0});
        printf(
            "  Input: [%.1f, %.1f], Predicted: %.1f, Actual: %.1f\n",
            (double) tensor_get_element(perceptron->x, (uint32_t[]){i, 0}),
            (double) tensor_get_element(perceptron->x, (uint32_t[]){i, 1}),
            (double) predicted,
            (double) actual
        );
    }

    // Clean up memory
    free_parameters(params);
    free_perceptron(perceptron);

    return 0;
}
