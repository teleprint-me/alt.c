/**
 * @file examples/models/similarity.c
 * @brief Create a function to compute the Cosine Similarity for any string of text
 */

#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Vector {
    float* data;
    size_t width;
} Vector;

typedef struct VectorArgs {
    size_t thread_count;
    size_t thread_id;
    Vector* a;
    Vector* b;
} VectorArgs;

void random_seed(uint32_t seed);

float random_linear(void);
void random_linear_init_vector(Vector* vector);

Vector* vector_create(size_t width);
void vector_free(Vector* vector);

void random_seed(uint32_t seed) {
    srand(seed);
}

// Linear initialization [0, 1]
float random_linear(void) {
    return (float) rand() / (float) RAND_MAX;
}

// Initializes a flat vector
void random_linear_init_vector(Vector* vector) {
    if (!(vector->width > 0 && vector->width < UINT32_MAX)) {
        return;
    }

    for (uint32_t i = 0; i < vector->width; i++) {
        vector->data[i] = random_linear();
    }
}

Vector* vector_create(size_t length) {
    if (length == 0) {
        return NULL;
    }

    Vector* vector = malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->data = malloc(sizeof(float) * length);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    vector->width = length;
    return vector;
}

void vector_free(Vector* vector) {
    free(vector->data);
    free(vector);
}

// Computes the dot product between two vectors
float vector_dot(Vector* a, Vector* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    if (a->width != b->width) {
        return 0;
    }

    float dot_product = 0;
    for (size_t i = 0; i < a->width; i++) {
        dot_product += a->data[i] * b->data[i];
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
float vector_cosine_similarity(Vector* a, Vector* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    if (a->width != b->width) {
        return 0;
    }

    float dot_product = vector_dot(a, b);
    float magnitude1 = vector_magnitude(a);
    float magnitude2 = vector_magnitude(b);

    if (magnitude1 == 0 || magnitude2 == 0) {
        return 0;
    }

    return dot_product / (magnitude1 * magnitude2);
}

int main() {
    // Create two vectors
    Vector* a = vector_create(10);
    Vector* b = vector_create(10);

    // Initialize vectors with random values
    random_linear_init_vector(a);
    random_linear_init_vector(b);

    // Compute cosine similarity
    float similarity = vector_cosine_similarity(a, b);

    printf("Cosine similarity: %f\n", similarity);

    // Free memory
    vector_free(a);
    vector_free(b);

    return 0;
}
