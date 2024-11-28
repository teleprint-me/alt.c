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

#include "logger.h"
#include "tensors.h"

Tensor* tensor_create(FlexArray* shape, uint32_t rank, DataTypeId id) {
    if (!shape || !shape->data || rank == 0) {
        LOG_ERROR("Invalid Tensor shape or rank provided.\n");
        return NULL;
    }

    const DataType* type = data_type_get(id);
    if (!type) {
        LOG_ERROR("Invalid Tensor data type provided.\n");
        return NULL;
    }

    Tensor* tensor = (Tensor*) malloc(sizeof(Tensor));
    if (!tensor) {
        LOG_ERROR("Failed to allocate memory for Tensor structure.\n");
        return NULL;
    }

    tensor->shape = shape; // Tensor takes ownership of the shape
    tensor->rank = rank;
    tensor->type = type;

    uint32_t total_size = 1;
    for (uint32_t i = 0; i < rank; i++) {
        uint32_t dim = ((uint32_t*) shape->data)[i];
        if (dim == 0) {
            LOG_ERROR("Zero dimension detected in tensor shape at dimension %u.\n", i);
            flex_array_free(shape); // Free the shape if tensor creation fails
            free(tensor);
            return NULL;
        }
        total_size *= dim;
    }

    tensor->data = malloc(total_size * type->size);
    if (!tensor->data) {
        LOG_ERROR("Failed to allocate memory for Tensor data.\n");
        flex_array_free(shape); // Free the shape if tensor creation fails
        free(tensor);
        return NULL;
    }

    memset(tensor->data, 0, total_size * type->size); // Zero-initialize data
    return tensor;
}

void tensor_free(Tensor* tensor) {
    if (tensor) {
        if (tensor->shape) {
            flex_array_free(tensor->shape); // Free the shape
        }
        if (tensor->data) {
            free(tensor->data); // Free the tensor data
        }
        free(tensor); // Free the tensor structure
    }
}

TensorState tensor_compute_shape(const Tensor* tensor, uint32_t* size) {
    // Validate inputs
    if (!tensor || !tensor->shape || !tensor->shape->data || !size) {
        LOG_ERROR("Invalid tensor, shape, or size provided.\n");
        return TENSOR_INVALID_SHAPE; // Invalid arguments
    }

    // Initialize size to 1 (multiplicative identity)
    *size = 1;

    // Multiply all dimensions in the shape
    for (uint32_t i = 0; i < tensor->rank; ++i) {
        uint32_t dim = ((uint32_t*) tensor->shape->data)[i];
        if (dim == 0) {
            LOG_ERROR("Zero dimension detected in tensor shape at dimension %u.\n", i);
            return TENSOR_INVALID_SHAPE; // Reject zero dimensions
        }
        if (*size > UINT32_MAX / dim) {
            LOG_ERROR("Index overflow detected during computation.\n");
            return TENSOR_ERROR; // Overflow prevention
        }
        *size *= dim;
    }

    return TENSOR_SUCCESS;
}

TensorState tensor_compute_index(const Tensor* tensor, const FlexArray* indices, uint32_t* index) {
    // Validate inputs
    if (!tensor || !tensor->shape || !tensor->shape->data) {
        LOG_ERROR("Invalid tensor or shape provided.\n");
        return TENSOR_INVALID_SHAPE;
    }
    if (!indices || !indices->data || !index) {
        LOG_ERROR("Invalid indices or output pointer provided.\n");
        return TENSOR_INVALID_INDICES;
    }
    if (tensor->rank != indices->length) {
        LOG_ERROR(
            "Rank mismatch: tensor rank=%u, indices length=%u.\n", tensor->rank, indices->length
        );
        return TENSOR_INVALID_RANK;
    }

    uint32_t flat_index = 0, stride = 1;
    for (int i = tensor->rank - 1; i >= 0; --i) {
        uint32_t offset = ((uint32_t*) indices->data)[i];
        uint32_t dim = ((uint32_t*) tensor->shape->data)[i];

        if (dim == 0) {
            LOG_ERROR("Zero dimension detected in tensor shape at dimension %u.", i);
            return TENSOR_INVALID_SHAPE;
        }
        if (offset >= dim) {
            LOG_WARN("Index out of bounds in dimension %u: offset=%u, dim=%u.", i, offset, dim);
            return TENSOR_OUT_OF_BOUNDS;
        }
        if (flat_index > UINT32_MAX - (offset * stride)) {
            LOG_ERROR("Index overflow detected during computation.");
            return TENSOR_ERROR; // Prevent overflow
        }

        flat_index += offset * stride;
        stride *= dim;
    }

    *index = flat_index;
    return TENSOR_SUCCESS;
}

TensorState tensor_compute_array(const Tensor* tensor, FlexArray* indices, const uint32_t index) {
    // Validate inputs
    if (!tensor || !tensor->shape || !tensor->shape->data) {
        LOG_ERROR("Invalid tensor or shape provided.\n");
        return TENSOR_INVALID_SHAPE;
    }
    if (!indices || !indices->data) {
        LOG_ERROR("Invalid indices or output pointer provided.\n");
        return TENSOR_INVALID_INDICES;
    }
    if (tensor->rank != indices->length) {
        LOG_ERROR(
            "Rank mismatch: tensor rank=%u, indices length=%u.\n", tensor->rank, indices->length
        );
        return TENSOR_INVALID_RANK;
    }

    // Compute the total size of the tensor (max index)
    uint32_t max_index = 0;
    TensorState state = tensor_compute_shape(tensor, &max_index);
    if (state != TENSOR_SUCCESS) {
        return state; // Propagate error from tensor_compute_shape
    }

    if (index >= max_index) {
        LOG_WARN("Flat index out of bounds: index=%u, max_index=%u.", index, max_index);
        return TENSOR_OUT_OF_BOUNDS;
    }

    // Compute multi-dimensional indices
    uint32_t flat_index_remaining = index;
    for (int i = tensor->rank - 1; i >= 0; --i) {
        uint32_t dim = ((uint32_t*)tensor->shape->data)[i];
        ((uint32_t*)indices->data)[i] = flat_index_remaining % dim;
        flat_index_remaining /= dim;
    }

    return TENSOR_SUCCESS;
}

TensorState tensor_get_element(Tensor* tensor, const FlexArray* indices, void* value) {
    if (!tensor || !value) {
        LOG_ERROR("Invalid tensor or output pointer provided.");
        return TENSOR_ERROR;
    }

    uint32_t flat_index;
    TensorState state = tensor_compute_index(tensor, indices, &flat_index);
    if (state != TENSOR_SUCCESS) {
        return state;
    }

    memcpy(value, (char*)tensor->data + flat_index * tensor->type->size, tensor->type->size);
    return TENSOR_SUCCESS;
}

TensorState tensor_set_element(Tensor* tensor, const FlexArray* indices, void* value) {
    if (!tensor || !value) {
        LOG_ERROR("Invalid tensor or input pointer provided.");
        return TENSOR_ERROR;
    }

    uint32_t flat_index;
    TensorState state = tensor_compute_index(tensor, indices, &flat_index);
    if (state != TENSOR_SUCCESS) {
        return state;
    }

    memcpy((char*)tensor->data + flat_index * tensor->type->size, value, tensor->type->size);
    return TENSOR_SUCCESS;
}
