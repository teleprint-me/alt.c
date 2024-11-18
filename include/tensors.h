/**
 * @file include/tensors.h
 *
 * @brief 32-bit implementation for tensor management.
 *
 * @note This implementation is intentionally kept as simple and minimalistic as possible.
 */

#ifndef ALT_TENSORS_H
#define ALT_TENSORS_H

// @brief Simplifies usage by mapping the tensor_compute_flat_index function call.
#define TENSOR_IDX(tensor, indices) tensor_compute_flat_index((tensor), (indices))

typedef struct Tensor {
    float* data; // Raw data stored in a flattened array.
    unsigned int* shape; // Shape array (e.g., [4, 2] for 4x2 matrix).
    unsigned int rank; // Number of dimensions.
} Tensor;

// @brief Allocates memory for the tensor, its shape, and its data.
Tensor* tensor_create(unsigned int* shape, unsigned int rank);
// @brief Safely deallocates memory for the tensor, including its shape and data arrays.
void tensor_free(Tensor* tensor);

// @brief Converts multi-dimensional indices to a flat array index.
unsigned int tensor_compute_flat_index(const Tensor* tensor, unsigned int* indices);
// @brief Converts a flat index back to multi-dimensional indices.
void tensor_compute_multi_indices(const Tensor* tensor, unsigned int* indices, unsigned int flat_index);

// @brief Retrieves an element by its indices.
float tensor_get_element(Tensor* tensor, unsigned int* indices);
// @brief Sets an element at a specific set of indices.
void tensor_set_element(Tensor* tensor, unsigned int* indices, float value);

#endif // ALT_TENSORS_H
