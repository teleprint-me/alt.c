/**
 * @file examples/models/similarity.c
 * @brief A simple embedding model leveraging basic NLP techniques.
 *
 * ## Overview
 * This implementation focuses on creating a naive embedding model
 * using the simplest approach: Cosine Similarity. 
 * While advanced methods like Word2Vec and GloVe are common in NLP,
 * this example intentionally avoids their complexity.
 *
 * ## Context
 * Techniques such as Continuous Bag of Words (CBOW) and Skip-gram
 * are often employed for embeddings, but they are not the focus here.
 * Instead, the goal is to demonstrate a minimalistic approach to 
 * embedding models.
 *
 * ## Note
 * This implementation is primarily educational and is not concerned 
 * with the intricacies or optimizations of more sophisticated methods.
 */

#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef N_THREADS
    #define N_THREADS sysconf(_SC_NPROCESSORS_ONLN)
#endif // N_THREADS

/// @brief Vector structures

typedef struct Vector {
    uint32_t width; // Number of elements
    float* data; // Pointer to data
} Vector;

// Arguments for thread-based vector operations
typedef struct VectorScalarArgs {
    uint32_t thread_id; // Thread ID
    uint32_t thread_count; // Total number of threads
    Vector* a_in; // Input vector 1
    Vector* b_in; // Input vector 2 (or NULL for single-vector operations)
    float partial_out; // Partial result (dot product or magnitude)
} VectorScalarArgs;

/// @brief Matrix structures

typedef struct Matrix {
    uint32_t width; // embedding dimensions
    uint32_t height; // vocabulary size
    float* data; // flat matrix
} Matrix;

/// @brief Random: Initialize a vector or matrix with normalized values.
/// @note This is intentionally kept as simple as possible.

// Initializes the random seed
void random_seed(uint32_t seed) {
    srand(seed);
}

// Generates a random float in the range [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Initializes a flat vector with random values
int random_linear_init_vector(Vector* vector) {
    if (!(vector->width > 0 && vector->width < UINT32_MAX)) {
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < vector->width; i++) {
        vector->data[i] = random_linear();
    }

    return EXIT_SUCCESS;
}

// Initializes a flat matrix with random values
int random_linear_init_matrix(Matrix* matrix) {
    uint32_t size = matrix->height * matrix->width;
    if (!(size > 0 && size < UINT32_MAX)) {
        return EXIT_FAILURE;
    }

    for (uint32_t i = 0; i < size; i++) {
        matrix->data[i] = random_linear();
    }

    return EXIT_SUCCESS;
}

Vector* vector_create(size_t width) {
    if (0 == width) {
        return NULL;
    }

    Vector* vector = (Vector*) malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->width = width;
    vector->data = malloc(sizeof(float) * vector->width);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    int result = random_linear_init_vector(vector);
    if (EXIT_FAILURE == result) {
        free(vector->data);
        free(vector);
        return NULL;
    }

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

Matrix* matrix_create(uint32_t height, uint32_t width) {
    if (0 == width) { // it's okay if height is 0.
        return NULL;
    }

    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    if (!matrix) {
        return NULL;
    }

    matrix->height = height;
    matrix->width = width;
    matrix->data = (float*) malloc(sizeof(float) * matrix->height * matrix->width);
    if (!matrix->data) {
        free(matrix);
        return NULL;
    }

    int result = random_linear_init_matrix(matrix);
    if (EXIT_FAILURE == result) {
        free(matrix->data);
        free(matrix);
        return NULL;
    }

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

// Thread function for partial dot product
void* vector_dot_partial(void* args) {
    VectorScalarArgs* v_args = (VectorScalarArgs*) args;
    size_t start = (v_args->a_in->width / v_args->thread_count) * v_args->thread_id;
    size_t end = 0;
    if (v_args->thread_id == v_args->thread_count - 1) {
        end = v_args->a_in->width;
    } else {
        end = start + (v_args->a_in->width / v_args->thread_count);
    }

    v_args->partial_out = 0;
    for (size_t i = start; i < end; i++) {
        v_args->partial_out += v_args->a_in->data[i] * v_args->b_in->data[i];
    }

    return NULL;
}

// Parallel dot product
float vector_dot(Vector* a, Vector* b, float bias) {
    if (!a || !b || a->width != b->width || N_THREADS == 0) {
        fprintf(stderr, "Error: Invalid parameters for vector_dot.\n");
        return 0;
    }

    const size_t n_threads = N_THREADS;

    pthread_t threads[n_threads];
    VectorScalarArgs args[n_threads];

    // Initialize thread arguments
    for (size_t i = 0; i < n_threads; i++) {
        args[i] = (VectorScalarArgs
        ){.thread_id = i, .thread_count = n_threads, .a_in = a, .b_in = b, .partial_out = 0};
        pthread_create(&threads[i], NULL, vector_dot_partial, &args[i]);
    }

    // Wait for threads and combine results
    float dot_product = bias; // start with bias
    for (size_t i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
        dot_product += args[i].partial_out;
    }

    return dot_product;
}

// Computes the magnitude (Euclidean norm) of a vector
float vector_magnitude(Vector* vector) {
    if (vector == NULL) {
        return 0;
    }

    float magnitude = 0;
    for (size_t i = 0; i < vector->width; i++) {
        magnitude += vector->data[i] * vector->data[i];
    }

    return sqrtf(magnitude);
}

// Computes the cosine similarity between two vectors
float vector_cosine_similarity(Vector* a, Vector* b, float bias) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    if (a->width != b->width) {
        return 0;
    }

    float dot_product = vector_dot(a, b, bias);
    float magnitude1 = vector_magnitude(a);
    float magnitude2 = vector_magnitude(b);

    if (magnitude1 == 0 || magnitude2 == 0) {
        return 0;
    }

    return dot_product / (magnitude1 * magnitude2);
}

// Example tokenization function
void tokenize(const char* text) {
    const char* delimiters = " .,!?;:\"()";
    char* text_copy = strdup(text); // Make a mutable copy
    char* token = strtok(text_copy, delimiters);

    while (token) {
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiters);
    }

    free(text_copy);
}

int main(void) {
    // Create two vectors
    Vector* a = vector_create(32000);
    Vector* b = vector_create(32000);

    // Initialize vectors with random values
    random_linear_init_vector(a);
    random_linear_init_vector(b);

    // Compute cosine similarity
    float similarity = vector_cosine_similarity(a, b, 0.0f); // no bias

    printf("Cosine similarity: %f\n", (double) similarity);

    // Free memory
    vector_free(a);
    vector_free(b);

    return 0;
}
