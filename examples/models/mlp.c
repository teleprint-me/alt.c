/**
 * @file examples/scratchpad/mlp.c
 *
 * @brief A Multi-layer perceptron completely from scratch in pure C.
 * 
 * The goal is to train a MLP model to recognize english characters.
 * 
 * - a-z
 * - A-Z
 * - 0-9
 * - Punctuation
 * - etc.
 * 
 * Requirements:
 * - Hyperparameters should be configurable at runtime.
 * - The dataset can be automatically generated at runtime.
 * - The model should be as simple and straightforward as reasonably possible.
 * - Helper functions are to be utilized to ensure code is modular, readable, and maintainable.
 * - Multi-threading can be considered if single-threading is too slow.
 * - Use 32-bit floats for enhanced learning.
 * - Use one-hot encoding for probabilities.
 * - Use SDG to keep the implementation simple and straightforward.
 * 
 * Keep it lightweight and simple, don't assume or anticipate anything, and reiterate as needed!
 * 
 * Oh, and have fun!
 */

#include "random.h"

#include <stdio.h>

// Can be used for anything requiring an array and array length
// e.g. an object requiring sample related information.
typedef struct Vector {
    uint32_t width; // cols
    float* data; // flat array
} Vector;

// Useful for handling the weights in the model
typedef struct Matrix {
    uint32_t width; // cols
    uint32_t height; // rows
    float* data; // flat matrix
} Matrix;

// Useful for SDG and Epoch metrics
typedef struct ErrorTracker {
    Matrix** layer_errors; /**< Per-layer errors for the current sample. */
    Vector* epoch_errors; /**< Aggregate errors for each epoch. */
} ErrorTracker;

// User input model hyperparameters for runtime configuration
typedef struct Parameters {
    float error_threshold; /**< Threshold for early stopping. */
    float learning_rate; /**< Learning rate for gradient descent. */
    uint32_t n_threads; /**< Number of CPU threads to use. */
    uint32_t n_epochs; /**< Number of training epochs. */
    uint32_t n_layers; /**< Number of layers (connections). */
    Vector* layer_sizes; /**< Sizes of each layer (input, hidden, output). */
} Parameters;

// Model layers
typedef struct Layer {
    Matrix* weights;
    Vector* biases;
    Vector* activations;
    Vector* gradients;
} Layer;

// Multi-layer perceptron
typedef struct MLP {
    Parameters* params;
    ErrorTracker* tracker;
    Layer* layers;
} MLP;

Vector* vector_create(uint32_t width);
void vector_free(Vector* vector);

// Useful for managing one-dimensional arrays
Vector* vector_create(uint32_t width) {
    Vector* vector = (Vector*) malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->data = (float*) malloc(sizeof(float) * width);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    for (uint32_t i = 0; i < width; i++) {
        vector->data[i] = random_linear(); // normalized values between 0 and 1
    }

    vector->width = width;

    return vector;
}

void vector_free(Vector* vector) {
    if (vector) {
        if (vector->data) {
            free(vector->data);
        }
        free(vector);
    }
}

// Simplifies operations related to weights
Matrix* matrix_create(uint32_t height, uint32_t width) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    if (!matrix) {
        return NULL;
    }

    matrix->data = (float*) malloc(sizeof(float) * height * width);
    if (!matrix->data) {
        free(matrix);
        return NULL;
    }

    for (uint32_t i = 0; i < (height * width); i++) {
        matrix->data[i] = random_linear(); // normalized values between 0 and 1
    }

    matrix->width = width;
    matrix->height = height;

    return matrix;
}

void matrix_free(Matrix* matrix) {
    if (matrix) {
        if (matrix->data) {
            free(matrix->data);
        }
        free(matrix);
    }
}

// Utilities for getting and setting elements for matrices to minimize errors
float matrix_get_element(const Matrix* matrix, const uint32_t row, const uint32_t col) {
    if (row >= matrix->height || col >= matrix->width) {
        return NAN;
    }
    return matrix->data[row * matrix->width + col];
}

int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, float value) {
    if (row >= matrix->height || col >= matrix->width) {
        return 1; // error
    }
    matrix->data[row * matrix->width + col] = value;
    return 0; // success
}

// Utility for simplifying required transpose operations
Matrix* matrix_transpose(Matrix* matrix) {
    // Create a new matrix with swapped rows and cols
    Matrix* T = matrix_create(matrix->width, matrix->height);
    if (!T) {
        return NULL;
    }

    // Populate the transposed matrix
    for (uint32_t row = 0; row < matrix->height; row++) {
        for (uint32_t col = 0; col < matrix->width; col++) {
            // Element at (i, j) in original becomes (j, i) in transposed
            T->data[col * matrix->height + row] = matrix->data[row * matrix->width + col];
        }
    }

    return T;
}

// Utilities for matrix-friendly stdout operations
void matrix_print_flat(Matrix* matrix) {
    for (uint32_t i = 0; i < (matrix->height * matrix->width); i++) {
        printf("%.6f ", (double) matrix->data[i]);
    }
    printf("\n");
}

void matrix_print_grid(Matrix* matrix) {
    for (uint32_t i = 0; i < matrix->height; i++) {
        for (uint32_t j = 0; j < matrix->width; j++) {
            printf("%.6f ", (double) matrix->data[i * matrix->width + j]);
        }
        printf("\n");
    }
}

int main(void) {
    random_seed(1337); // Fix seed for reproducibility

    // Create a randomly initialized 3x4 matrix
    Matrix* matrix = matrix_create(3, 4);
    printf("Flat Matrix:\n");
    matrix_print_flat(matrix);
    printf("\n");

    printf("Original Matrix:\n");
    matrix_print_grid(matrix);

    // Transpose the matrix
    Matrix* transposed = matrix_transpose(matrix);
    if (transposed) {
        printf("\nTransposed Matrix:\n");
        matrix_print_grid(transposed);
    }

    // Free both matrices
    matrix_free(matrix);
    matrix_free(transposed);

    return 0;
}
