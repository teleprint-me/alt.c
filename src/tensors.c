/**
 * @file src/tensors.c
 */

#include <string.h>

#include "tensors.h"

Tensor* create_tensor(unsigned int* shape, unsigned int n_dims) {
    // Allocate memory to the tensor
    Tensor* tensor = malloc(sizeof(Tensor));

    // Allocate memory to the tensor shape
    tensor->shape = malloc(n_dims * sizeof(unsigned int));
    memcpy(tensor->shape, shape, n_dims * sizeof(unsigned int));

    // Set the number of dimensions in shape
    tensor->n_dims = n_dims;

    // Calculate the number of data elements
    unsigned int total_size = 1;
    for (unsigned int i = 0; i < n_dims; i++) {
        total_size *= shape[i];
    }

    // Zero-initialize tensor data
    tensor->data = malloc(total_size * sizeof(float));
    for (unsigned int i = 0; i < total_size; i++) {
        tensor->data[i] = 0.0f;
    }

    return tensor;
}

void free_tensor(Tensor* tensor) {
    if (tensor) {
        if (tensor->shape) {
            free(tensor->shape);
        }
        if (tensor->data) {
            free(tensor->data);
        }
        free(tensor);
    }
}

unsigned int tensor_index(const Tensor* tensor, unsigned int* indices) {
    unsigned int offset = 0;
    unsigned int stride = 1;

    // Compute the offset by iterating backward over dimensions
    for (int i = tensor->n_dims - 1; i >= 0; --i) {
        offset += indices[i] * stride;
        stride *= tensor->shape[i];
    }

    return offset;
}

float tensor_get(Tensor* tensor, unsigned int* indices) {
    return tensor->data[TENSOR_IDX(tensor, indices)];
}

void tensor_set(Tensor* tensor, unsigned int* indices, float value) {
    tensor->data[TENSOR_IDX(tensor, indices)] = value;
}

float tensor_dot_product(float* a, float* b, unsigned int length) {
    float result = 0.0;
    for (unsigned int i = 0; i < length; i++) {
        result += a[i] * b[i];
    }
    return result;
}

float tensor_row_dot_product(Tensor* tensor, unsigned int row_a, unsigned int row_b) {
    unsigned int n_cols = tensor->shape[1];
    float* a = &tensor->data[row_a * n_cols];
    float* b = &tensor->data[row_b * n_cols];
    return tensor_dot_product(a, b, n_cols);
}
