/**
 * @file examples/models/similarity.c
 * @brief Create a function to compute the Cosine Similarity for any string of text
 * @todo Add cpu parallelism using posix threads
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

typedef struct Vector {
    size_t width; // Number of elements
    float* data; // Pointer to data
} Vector;

// Arguments for thread-based vector operations
typedef struct VectorScalarArgs {
    size_t thread_id; // Thread ID
    size_t thread_count; // Total number of threads
    Vector* a_in; // Input vector 1
    Vector* b_in; // Input vector 2 (or NULL for single-vector operations)
    float partial_out; // Partial result (dot product or magnitude)
} VectorScalarArgs;

void random_seed(uint32_t seed);
float random_linear(void);
void random_linear_init_vector(Vector* vector);

Vector* vector_create(size_t width);
void vector_free(Vector* vector);

// Initializes the random seed
void random_seed(uint32_t seed) {
    srand(seed);
}

// Generates a random float in the range [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Initializes a flat vector with random values
void random_linear_init_vector(Vector* vector) {
    if (!(vector->width > 0 && vector->width < UINT32_MAX)) {
        return;
    }

    for (size_t i = 0; i < vector->width; i++) {
        vector->data[i] = random_linear();
    }
}

Vector* vector_create(size_t width) {
    if (width == 0) {
        return NULL;
    }

    Vector* vector = malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->data = malloc(sizeof(float) * width);
    if (!vector->data) {
        free(vector);
        return NULL;
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
