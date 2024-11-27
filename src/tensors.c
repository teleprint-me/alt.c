/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/tensors.c
 *
 * @brief Implementation for tensor management.
 *
 * @note This implementation is intentionally kept as simple and minimalistic as possible.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tensors.h"

Tensor* tensor_create(FlexArray* shape, uint32_t rank, DataTypeId id) {
    if (!shape || !shape->data || rank == 0) {
        return NULL;
    }

    const DataType* type = data_type_get(id);
    if (!type) {
        return NULL;
    }

    Tensor* tensor = (Tensor*)malloc(sizeof(Tensor));
    if (!tensor) {
        return NULL;
    }

    tensor->shape = shape;
    tensor->rank = rank;
    tensor->type = type;

    // Compute total size and allocate tensor data
    uint32_t total_size = 1;
    for (uint32_t i = 0; i < rank; i++) {
        uint32_t dim = ((uint32_t*)shape->data)[i];
        if (dim == 0) { // Validate dimension
            flex_array_free(tensor->shape);
            free(tensor);
            return NULL;
        }
        total_size *= dim;
    }

    tensor->data = malloc(total_size * type->size);
    if (!tensor->data) {
        flex_array_free(tensor->shape);
        free(tensor);
        return NULL;
    }

    return tensor;
}

void tensor_free(Tensor* tensor) {
    if (tensor) {
        if (tensor->shape) {
            flex_array_free(tensor->shape);
        }
        if (tensor->data) {
            free(tensor->data);
        }
        free(tensor);
    }
}

TensorState tensor_compute_shape(const Tensor* tensor, unsigned int* size) {
    // Step 1: Validate inputs
    if (!tensor || !tensor->shape || !tensor->shape->data || !size) {
        return TENSOR_ERROR; // Invalid arguments
    }

    // Step 2: Initialize size to 1 (multiplicative identity)
    *size = 1;

    // Step 3: Multiply all dimensions in the shape
    for (unsigned int i = 0; i < tensor->rank; ++i) {
        *size *= ((unsigned int*)tensor->shape->data)[i];
    }

    return TENSOR_SUCCESS;
}

TensorState tensor_compute_index(const Tensor* tensor, const FlexArray* indices, unsigned int* index) {
    if (!tensor || !indices || !index || !indices->data) {
        return TENSOR_ERROR;
    }
    if (indices->length != tensor->rank) {
        return TENSOR_INVALID_RANK;
    }

    unsigned int flat_index = 0, stride = 1;
    for (int i = tensor->rank - 1; i >= 0; --i) {
        unsigned int offset = ((unsigned int*)indices->data)[i];
        if (offset >= ((unsigned int*)tensor->shape->data)[i]) {
            return TENSOR_OUT_OF_BOUNDS;
        }
        flat_index += offset * stride;
        stride *= ((unsigned int*)tensor->shape->data)[i];
    }

    *index = flat_index;
    return TENSOR_SUCCESS;
}

TensorState tensor_compute_array(const Tensor* tensor, FlexArray* indices, const unsigned int index) {
    // Validate inputs
    if (!tensor || !indices || !indices->data) {
        return TENSOR_ERROR; // Null pointers are not allowed
    }
    if (indices->length != tensor->rank) {
        return TENSOR_INVALID_RANK; // Rank mismatch
    }

    // Ensure the flat index is within bounds
    unsigned int max_index = 1;
    for (unsigned int i = 0; i < tensor->rank; ++i) {
        max_index *= ((unsigned int*)tensor->shape->data)[i];
    }
    if (index >= max_index) {
        return TENSOR_OUT_OF_BOUNDS; // Index exceeds tensor size
    }

    // Compute multi-dimensional indices
    unsigned int remaining_index = index;
    for (int i = tensor->rank - 1; i >= 0; --i) {
        ((unsigned int*)indices->data)[i] = remaining_index % ((unsigned int*)tensor->shape->data)[i];
        remaining_index /= ((unsigned int*)tensor->shape->data)[i];
    }

    return TENSOR_SUCCESS;
}

TensorState tensor_get_element(Tensor* tensor, const FlexArray* indices, void* value) {
    unsigned int flat_index;
    TensorState state = tensor_compute_index(tensor, indices, &flat_index);
    if (state != TENSOR_SUCCESS) {
        return state;
    }
    memcpy(value, (char*)tensor->data + flat_index * sizeof(float), sizeof(float));
    return TENSOR_SUCCESS;
}

TensorState tensor_set_element(Tensor* tensor, const FlexArray* indices, void* value) {
    unsigned int flat_index;
    TensorState state = tensor_compute_index(tensor, indices, &flat_index);
    if (state != TENSOR_SUCCESS) {
        return state;
    }
    memcpy((char*)tensor->data + flat_index * sizeof(float), value, sizeof(float));
    return TENSOR_SUCCESS;
}
