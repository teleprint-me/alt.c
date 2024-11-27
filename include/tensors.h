/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/tensors.h
 *
 * @brief N-Dimensional tensors.
 */

#ifndef ALT_TENSORS_H
#define ALT_TENSORS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data_types.h"
#include "flex_array.h"

typedef enum TensorState {
    TENSOR_SUCCESS, /**< Operation succeeded */
    TENSOR_ERROR, /**< General error */
    TENSOR_INVALID_RANK, /**< Rank mismatch between tensor and indices */
    TENSOR_RESIZE, /**< Resize operation performed */
    TENSOR_TRANSPOSE, /**< Transpose operation performed */
    TENSOR_OUT_OF_BOUNDS, /**< Index out of bounds */
    TENSOR_MEMORY_ALLOCATION_FAILED /**< Memory allocation failure */
} TensorState;

typedef struct Tensor {
    uint32_t rank; /**< Number of dimensions */
    void* data; /**< N-dimensional data stored as a flattened array */
    FlexArray* shape; /**< Shape array defining dimensions */
    DataType type; /**< Data type of the tensor elements (e.g., float, double, int) */
} Tensor;

Tensor* tensor_create(FlexArray* shape, uint32_t rank, DataTypeId id);
void tensor_free(Tensor* tensor);

TensorState tensor_compute_shape(const Tensor*, FlexArray* shape);
TensorState tensor_compute_index(const Tensor* tensor, const FlexArray* indices, uint32_t* index);
TensorState tensor_compute_array(const Tensor* tensor, FlexArray* indices, const uint32_t* index);

TensorState tensor_get_element(Tensor* tensor, const FlexArray* indices, void* value);
TensorState tensor_set_element(Tensor* tensor, const FlexArray* indices, void* value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_TENSORS_H
