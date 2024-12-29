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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface/logger.h"

#include "tensors.h"

Tensor* tensor_create(DataTypeId id, uint32_t rank, uint32_t* dimensions) {
    if (rank == 0) {
        LOG_ERROR("Rank must be greater than 0.\n");
        return NULL;
    }

    const DataType* type = data_type_get(id); // stack allocated ref
    if (!type) { // NULL on invalid id
        LOG_ERROR("%s: Invalid data type given using id=%u.\n", __func__, id);
        return NULL;
    }

    // Extract dimensions from variable arguments
    FlexArray* shape = tensor_create_shape(rank, dimensions);
    if (!shape) {
        LOG_ERROR("%s: Failed to create shape using rank=%u.\n", __func__, rank);
        return NULL; // tensor_create_shape logs errors
    }

    Tensor* tensor = (Tensor*) malloc(sizeof(Tensor));
    if (!tensor) {
        LOG_ERROR("%s: Failed to allocate memory for Tensor structure.\n", __func__);
        flex_array_free(shape); // Free the shape if tensor allocation fails
        return NULL;
    }

    // Tensor takes ownership of rank, shape, and type
    tensor->rank = rank;
    tensor->shape = shape;
    tensor->type = type;

    uint32_t size;
    if (tensor_compute_shape(tensor, &size) != TENSOR_SUCCESS) {
        LOG_ERROR("%s: Failed to compute tensor shape.\n", __func__);
        tensor_free(tensor); // Use centralized cleanup
        return NULL;
    }

    tensor->data = malloc(size * tensor->type->size);
    if (!tensor->data) {
        LOG_ERROR("%s: Failed to allocate memory for Tensor data.\n", __func__);
        tensor_free(tensor); // Use centralized cleanup
        return NULL;
    }

    memset(tensor->data, 0, size * tensor->type->size); // Zero-initialize data
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

FlexArray* tensor_create_shape(uint32_t rank, uint32_t* dimensions) {
    if (rank == 0) {
        LOG_ERROR("%s: Rank must be greater than 0.\n", __func__);
        return NULL;
    }

    for (uint32_t i = 0; i < rank; i++) {
        if (dimensions[i] == 0) {
            LOG_ERROR("%s: Dimension %u must be greater than 0.\n", __func__, i);
            return NULL;
        }
    }

    /// @note create a flex array helper function for aggregating this logic.
    /// @note it's not repeated often, but I can see this becoming the case.
    FlexArray* shape = flex_array_create(rank, TYPE_UINT32);
    if (!shape) {
        LOG_ERROR("%s: Failed to allocate FlexArray for shape with rank=%u.\n", __func__, rank);
        return NULL;
    }

    if (flex_array_set_bulk(shape, dimensions, rank) != FLEX_ARRAY_SUCCESS) {
        LOG_ERROR("%s: Failed to initialize FlexArray with dimensions.\n", __func__);
        flex_array_free(shape); // Free allocated shape
        return NULL;
    }

    return shape;
}

FlexArray* tensor_create_indices(uint32_t rank, uint32_t* dimensions) {
    if (rank == 0) {
        LOG_ERROR("Rank must be greater than 0.\n");
        return NULL;
    }

    FlexArray* indices = flex_array_create(rank, TYPE_UINT32);
    if (!indices) {
        LOG_ERROR("Failed to allocate memory for indices.\n");
        return NULL;
    }

    if (flex_array_set_bulk(indices, dimensions, rank) != FLEX_ARRAY_SUCCESS) {
        LOG_ERROR("Failed to set dimensions for indices.\n");
        flex_array_free(indices);
        return NULL;
    }

    return indices;
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
    uint32_t dimensions;
    for (uint32_t i = 0; i < tensor->rank; ++i) {
        flex_array_get(tensor->shape, i, &dimensions);
        LOG_DEBUG("%s: size=%u dimensions=%u\n", __func__, *size, dimensions);
        if (dimensions == 0) {
            LOG_ERROR("%s: Zero dimension detected in tensor shape at dimension %u.\n", __func__, i);
            return TENSOR_INVALID_SHAPE; // Reject zero dimension
        }
        if (*size > (uint32_t)(UINT32_MAX / 2)) { // Respect the upper boundary
            LOG_ERROR("Index overflow detected during computation.\n");
            return TENSOR_ERROR; // Overflow prevention
        }
        *size *= dimensions;
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
        uint32_t dimensions = ((uint32_t*) tensor->shape->data)[i];

        if (dimensions == 0) {
            LOG_ERROR("Zero dimension detected in tensor shape at dimension %u.", i);
            return TENSOR_INVALID_SHAPE;
        }
        if (offset >= dimensions) {
            LOG_WARN("Index out of bounds in dimension %u: offset=%u, dim=%u.", i, offset, dimensions);
            return TENSOR_OUT_OF_BOUNDS;
        }
        if (flat_index > UINT32_MAX - (offset * stride)) {
            LOG_ERROR("Index overflow detected during computation.");
            return TENSOR_ERROR; // Prevent overflow
        }

        flat_index += offset * stride;
        stride *= dimensions;
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
        uint32_t dimensions = ((uint32_t*)tensor->shape->data)[i];
        ((uint32_t*)indices->data)[i] = flat_index_remaining % dimensions;
        flat_index_remaining /= dimensions;
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

TensorState tensor_set_bulk(Tensor* tensor, const void* data) {
    if (!tensor || !tensor->data || !data) {
        LOG_ERROR("Invalid arguments provided to tensor_initialize.");
        return TENSOR_ERROR;
    }

    uint32_t size;
    TensorState state = tensor_compute_shape(tensor, &size);
    if (state != TENSOR_SUCCESS) {
        return state;
    }

    memcpy(tensor->data, data, size * tensor->type->size);
    return TENSOR_SUCCESS;
}
