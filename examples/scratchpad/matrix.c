/**
 * @file examples/scratchpad/matrix.c
 */

#include "random.h"

#include <stdio.h>

typedef struct Matrix {
    float* elements; // flat matrix
    uint32_t rows;
    uint32_t cols;
} Matrix;

Matrix* matrix_create(uint32_t rows, uint32_t cols) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    if (!matrix) {
        return NULL;
    }

    matrix->elements = (float*) malloc(sizeof(float) * rows * cols);
    if (!matrix->elements) {
        free(matrix);
        return NULL;
    }

    for (uint32_t i = 0; i < (rows * cols); i++) {
        matrix->elements[i] = random_linear(); // normalized values between 0 and 1
    }

    matrix->rows = rows;
    matrix->cols = cols;

    return matrix;
}

void matrix_free(Matrix* matrix) {
    if (matrix) {
        if (matrix->elements) {
            free(matrix->elements);
        }
        free(matrix);
    }
}

float matrix_get_element(const Matrix* matrix, const uint32_t row, const uint32_t col) {
    if (row >= matrix->rows || col >= matrix->cols) {
        return NAN;
    }
    return matrix->elements[row * matrix->cols + col];
}

int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, float value) {
    if (row >= matrix->rows || col >= matrix->cols) {
        return 1; // error
    }
    matrix->elements[row * matrix->cols + col] = value;
    return 0; // success
}

Matrix* matrix_transpose(Matrix* matrix) {
    // Create a new matrix with swapped rows and cols
    Matrix* T = matrix_create(matrix->cols, matrix->rows);
    if (!T) {
        return NULL;
    }

    // Populate the transposed matrix
    for (uint32_t row = 0; row < matrix->rows; row++) {
        for (uint32_t col = 0; col < matrix->cols; col++) {
            // Element at (i, j) in original becomes (j, i) in transposed
            T->elements[col * matrix->rows + row] = matrix->elements[row * matrix->cols + col];
        }
    }

    return T;
}

void matrix_print_flat(Matrix* matrix) {
    for (uint32_t i = 0; i < (matrix->rows * matrix->cols); i++) {
        printf("%.6f ", (double) matrix->elements[i]);
    }
    printf("\n");
}

void matrix_print_grid(Matrix* matrix) {
    for (uint32_t i = 0; i < matrix->rows; i++) {
        for (uint32_t j = 0; j < matrix->cols; j++) {
            printf("%.6f ", (double) matrix->elements[i * matrix->cols + j]);
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
