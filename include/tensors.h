/**
 * @file include/tensors.h
 */

#ifndef ALT_TENSORS_H
#define ALT_TENSORS_H

#define TENSOR_IDX(tensor, indices) tensor_index((tensor), (indices))

typedef struct Tensor {
    float* data; // Pointer to raw data
    unsigned int* shape; // Shape array (e.g., [4, 2] for 4x2 matrix)
    unsigned int n_dims; // Number of dimensions
} Tensor;

Tensor* create_tensor(unsigned int* shape, unsigned int n_dims);
void free_tensor(Tensor* tensor);

unsigned int tensor_index(const Tensor* tensor, unsigned int* indices);
float tensor_get(Tensor* tensor, unsigned int* indices);
void tensor_set(Tensor* tensor, unsigned int* indices, float value);

float tensor_dot_product(float* a, float* b, unsigned int length);

#endif // ALT_TENSORS_H
