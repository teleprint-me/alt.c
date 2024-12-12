/**
 * @file examples/models/mlp.c
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
#include "logger.h"

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

// Useful for SDG and Epoch metrics
typedef struct ErrorTracker {
    Matrix* layer_errors; /**< Per-layer errors for the current sample. */
    Vector* epoch_errors; /**< Aggregate errors for each epoch. */
} ErrorTracker;

// Multi-layer perceptron
typedef struct MLP {
    Parameters* params;
    ErrorTracker* tracker;
    Layer* layers;
} MLP;

// POSIX forward pass
typedef struct MLPForwardArgs {
    uint32_t thread_count; /**< Total number of threads. */
    uint32_t thread_id; /**< Thread ID for this thread. */
    Vector* inputs; /**< Input vector (e.g., MNIST pixels). */
    Vector* outputs; /**< Output activations vector. */
    Vector* biases; /**< Bias vector. */
    Matrix* weights; /**< Flattened weight matrix. */
} MLPForwardArgs;

// POSIX backward pass
typedef struct MLPBackwardArgs {
    uint32_t thread_count; /**< Total number of threads. */
    uint32_t thread_id; /**< Thread ID for this thread. */
    float learning_rate; /**< Learning rate for gradient descent. */
    Vector* targets; /**< One-hot encoded target vector. */
    Vector* inputs; /**< Input vector (e.g., MNIST pixels). */
    Vector* outputs; /**< Output gradients vector. */
    Vector* biases; /**< Bias vector. */
    Matrix* weights; /**< Flattened weight matrix. */
} MLPBackwardArgs;

typedef struct Dataset {
    uint32_t length;
    char* samples;
} Dataset;

// Utilities

void print_progress(char* title, float percentage, uint32_t width, char ch);

// Vector operations

Vector* vector_create(uint32_t width);
void vector_free(Vector* vector);

// Matrix operations

Matrix* matrix_create(uint32_t height, uint32_t width);
void matrix_free(Matrix* matrix);

float matrix_get_element(const Matrix* matrix, const uint32_t row, const uint32_t col);
int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, const float value);

Matrix* matrix_transpose(Matrix* matrix);

void matrix_print_flat(Matrix* matrix);
void matrix_print_grid(Matrix* matrix);

// Utilities

// @ref https://stackoverflow.com/a/36315819/20035933
void print_progress(char* title, float percentage, uint32_t width, char ch) {
    char bar[width + 1];
    for (uint32_t i = 0; i < width; i++) {
        bar[i] = ch;
    }
    bar[width] = '\0'; // Null-terminate the bar for safety

    uint32_t progress = (uint32_t) (percentage * 100 + 0.5f); // Round percentage
    uint32_t left = (uint32_t) (percentage * width + 0.5f); // Round bar width
    uint32_t right = width - left;

    printf("\r%s: %3u%% [%.*s%*s]", title, progress, left, bar, right, "");
    fflush(stdout);
}

// Vector operations

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

// Matrix operations

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

int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, const float value) {
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

// Dataset managements

Dataset* dataset_create(uint32_t start, uint32_t end) {
    if (start >= end) {
        return NULL; // end must be greater than start
    }

    Dataset* dataset = (Dataset*) malloc(sizeof(Dataset));

    dataset->length = end - start; // use the difference as the length
    dataset->samples = (char*) malloc(sizeof(char) * dataset->length);
    if (!dataset->samples) {
        return NULL;
    }

    // populate samples
    for (uint32_t i = 0, s = start; i < dataset->length && s < end; i++) {
        dataset->samples[i] = (char) i + s;
    }

    return dataset;
}

void dataset_free(Dataset* dataset) {
    if (dataset) {
        if (dataset->samples) {
            free(dataset->samples);
        }
        free(dataset);
    }
}

void dataset_print(Dataset* dataset) {
    for(uint32_t i = 0; i < dataset->length; i++) {
        char code = dataset->samples[i];
        printf("index=%d, code=%d, char=%c\n", i, code, code);
    }
}

int main(void) {
    random_seed(1337); // Fix seed for reproducibility

    Dataset* dataset = dataset_create(32, 127);

    dataset_print(dataset);

    dataset_free(dataset);

    return 0;
}
