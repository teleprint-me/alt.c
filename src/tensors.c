/**
 * @file src/tensors.c
 *
 * @brief 32-bit implementation for tensor management.
 *
 * @note This implementation is intentionally kept as simple and minimalistic as possible.
 */

#include <assert.h>
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
    assert(tensor != NULL && "Tensor must not be NULL.");
    assert(indices != NULL && "Indices array must not be NULL.");
    // assert(VECTOR_LEN(indices) >= tensor->rank && "Indices array length must match or exceed the tensor's rank.");

    unsigned int offset = 0;
    unsigned int stride = 1;

    for (int i = tensor->rank - 1; i >= 0; --i) {
        assert(indices[i] < tensor->shape[i] && "Index out of bounds.");
        offset += indices[i] * stride;
        stride *= tensor->shape[i];
    }

    return offset;
}

void tensor_compute_multi_indices(const Tensor* tensor, unsigned int* indices, unsigned int flat_index) {
    assert(tensor != NULL && "Tensor must not be NULL.");
    assert(indices != NULL && "Indices array must not be NULL.");
    // assert(VECTOR_LEN(indices) >= tensor->rank && "Indices array length must match or exceed the tensor's rank.");

    for (int dim = tensor->rank - 1; dim >= 0; --dim) {
        assert(flat_index < tensor->shape[dim] * tensor->shape[tensor->rank - 1] && "Flat index out of bounds.");
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
