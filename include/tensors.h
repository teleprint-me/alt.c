/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/tensors.h
 *
 * @brief Definitions and functions for working with N-dimensional tensors.
 */

#ifndef ALT_TENSORS_H
#define ALT_TENSORS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data_types.h"
#include "flex_array.h"

/**
 * @enum TensorState
 * @brief Return codes for tensor operations.
 */
typedef enum TensorState {
    TENSOR_SUCCESS, /**< Operation completed successfully */
    TENSOR_ERROR, /**< General error */
    TENSOR_INVALID_RANK, /**< Mismatch between tensor rank and indices */
    TENSOR_INVALID_SHAPE, /**< Invalid shape provided */
    TENSOR_INVALID_INDICES, /**< Invalid indices provided */
    TENSOR_RESIZE, /**< Resize operation completed */
    TENSOR_TRANSPOSE, /**< Transpose operation completed */
    TENSOR_OUT_OF_BOUNDS, /**< Indices out of bounds */
    TENSOR_MEMORY_ALLOCATION_FAILED /**< Memory allocation failed */
} TensorState;

/**
 * @struct Tensor
 * @brief Representation of an N-dimensional tensor.
 */
typedef struct Tensor {
    uint32_t rank; /**< Number of dimensions */
    FlexArray* shape; /**< Array defining the size of each dimension */
    const DataType* type; /**< Data type of the tensor elements */
    void* data; /**< Flattened array storing tensor elements */
} Tensor;

/**
 * @brief Creates a new tensor with the specified shape, rank, and data type.
 *
 * The tensor takes ownership of the provided shape and frees it on failure.
 *
 * @param shape FlexArray defining the shape of the tensor.
 * @param rank Number of dimensions.
 * @param id Data type identifier for the tensor elements.
 * @return Pointer to the created tensor or NULL on failure.
 */
Tensor* tensor_create(FlexArray* shape, uint32_t rank, DataTypeId id);

/**
 * @brief Creates a shape object dynamically.
 *
 * @param rank Number of dimensions.
 * @param dimensions Pointer to the dimensions array.
 * @return Pointer to the created FlexArray shape or NULL on failure.
 */
FlexArray* tensor_create_shape(uint32_t rank, const void* dimensions);

/**
 * @brief Frees a tensor and its owned resources.
 *
 * Frees the tensor's shape, data, and the tensor itself.
 *
 * @param tensor Pointer to the tensor to be freed.
 */
void tensor_free(Tensor* tensor);

/**
 * @brief Computes the size of the tensor based on its shape.
 *
 * @param tensor Pointer to the tensor.
 * @param size Pointer to store the computed size.
 * @return TensorState indicating the result of the operation.
 */
TensorState tensor_compute_shape(const Tensor* tensor, uint32_t* size);

/**
 * @brief Computes a flat index from multidimensional indices.
 *
 * @param tensor Pointer to the tensor.
 * @param indices FlexArray of indices.
 * @param index Pointer to store the computed flat index.
 * @return TensorState indicating the result of the operation.
 */
TensorState tensor_compute_index(const Tensor* tensor, const FlexArray* indices, uint32_t* index);

/**
 * @brief Computes multidimensional indices from a flat index.
 *
 * @param tensor Pointer to the tensor.
 * @param indices FlexArray to store the computed indices.
 * @param index Flat index.
 * @return TensorState indicating the result of the operation.
 */
TensorState tensor_compute_array(const Tensor* tensor, FlexArray* indices, const uint32_t index);

/**
 * @brief Retrieves an element from the tensor at the specified indices.
 *
 * @param tensor Pointer to the tensor.
 * @param indices FlexArray of indices.
 * @param value Pointer to store the retrieved value.
 * @return TensorState indicating the result of the operation.
 */
TensorState tensor_get_element(Tensor* tensor, const FlexArray* indices, void* value);

/**
 * @brief Sets an element in the tensor at the specified indices.
 *
 * @param tensor Pointer to the tensor.
 * @param indices FlexArray of indices.
 * @param value Pointer to the value to set.
 * @return TensorState indicating the result of the operation.
 */
TensorState tensor_set_element(Tensor* tensor, const FlexArray* indices, void* value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_TENSORS_H
