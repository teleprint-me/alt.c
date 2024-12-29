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

#include "interface/data_types.h"
#include "interface/flex_array.h"

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
 * @brief Creates a new tensor with the specified data type, rank, and shape.
 *
 * The tensor takes ownership of the provided shape and frees it on failure.
 *
 * @param id Data type identifier for the tensor elements.
 * @param rank Number of dimensions.
 * @param dimensions Array of length rank defining the shape.
 * @return Pointer to the created tensor or NULL on failure.
 */
Tensor* tensor_create(DataTypeId id, uint32_t rank, uint32_t* dimensions);

/**
 * @brief Frees a tensor and its owned resources.
 *
 * Frees the tensor's shape, data, and the tensor itself.
 *
 * @param tensor Pointer to the tensor to be freed.
 */
void tensor_free(Tensor* tensor);

/**
 * @brief Dynamically creates a shape object as a FlexArray.
 *
 * Shape dimensions must be 1 or greater.
 * 
 * @param rank Number of dimensions.
 * @param dimensions Array of length rank defining the shape.
 * @return Pointer to the created FlexArray shape or NULL on failure.
 */
FlexArray* tensor_create_shape(uint32_t rank, uint32_t* dimensions);

/**
 * @brief Dynamically creates a indices object as a FlexArray.
 *
 * Indice values must be 0 or greater.
 *
 * @param rank Number of indices.
 * @param dimensions Array of length rank defining the indices.
 * @return Pointer to the created FlexArray or NULL on failure.
 */
FlexArray* tensor_create_indices(uint32_t rank, uint32_t* dimensions);

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

/**
 * @brief Sets the tensor's data using the provided bulk data.
 *
 * Copies the provided data buffer into the tensor's internal storage.
 *
 * @param tensor Pointer to the Tensor to initialize.
 * @param data Pointer to the source data buffer.
 * @return TensorState indicating the result of the operation:
 *         - TENSOR_SUCCESS: Data successfully copied.
 *         - TENSOR_ERROR: Invalid arguments provided.
 *         - TENSOR_INVALID_SHAPE: Tensor size could not be computed.
 *
 * @details
 * - Ensures the provided `tensor` and `data` are valid.
 * - Calculates the size of the tensor's data using its shape and rank.
 * - Copies the data using `memcpy` for efficient bulk transfer.
 *
 * @note Assumes a dense tensor with compatible dimensions and a consistent memory layout.
 */
TensorState tensor_set_bulk(Tensor* tensor, const void* data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_TENSORS_H
