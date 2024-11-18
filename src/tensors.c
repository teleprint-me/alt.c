/**
 * @file src/tensors.c
 *
 * @brief 32-bit implementation for tensor management.
 *
 * @note This implementation is intentionally kept as simple and minimalistic as possible.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tensors.h"

Tensor* tensor_create(unsigned int* shape, unsigned int rank) {
    // Allocate memory to the tensor
    Tensor* tensor = malloc(sizeof(Tensor));

    // Allocate memory to the tensor shape
    tensor->shape = malloc(rank * sizeof(unsigned int));
    memcpy(tensor->shape, shape, rank * sizeof(unsigned int));

    // Set the number of dimensions in shape
    tensor->rank = rank;

    // Calculate the number of data elements
    unsigned int total_size = 1;
    for (unsigned int i = 0; i < rank; i++) {
        total_size *= shape[i];
    }

    // Zero-initialize tensor data
    tensor->data = malloc(total_size * sizeof(float));
    for (unsigned int i = 0; i < total_size; i++) {
        tensor->data[i] = 0.0f;
    }

    return tensor;
}

void tensor_free(Tensor* tensor) {
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

unsigned int tensor_compute_flat_index(const Tensor* tensor, unsigned int* indices) {
    unsigned int offset = 0;
    unsigned int stride = 1;

    // Compute the offset by iterating backward over dimensions
    for (int i = tensor->rank - 1; i >= 0; --i) {
        offset += indices[i] * stride;
        stride *= tensor->shape[i];
    }

    return offset;
}

void tensor_compute_multi_indices(const Tensor* tensor, unsigned int* indices, unsigned int flat_index) {
    for (int dim = tensor->rank - 1; dim >= 0; --dim) {
        indices[dim] = flat_index % tensor->shape[dim];
        flat_index /= tensor->shape[dim];
    }
}

float tensor_get_element(Tensor* tensor, unsigned int* indices) {
    return tensor->data[TENSOR_IDX(tensor, indices)];
}

void tensor_set_element(Tensor* tensor, unsigned int* indices, float value) {
    tensor->data[TENSOR_IDX(tensor, indices)] = value;
}
