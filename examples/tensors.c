/**
 * @file examples/tensor.c
 */

#include "tensors.h"

#include <stdio.h>

#define INPUT_RANK 2

/**
 * This example demonstrates how to create a 2D tensor, set an element at a specific index,
 * retrieve the value of an element at a specific index, and deallocate memory for the tensor.
 */
int main() {
    unsigned int shape[INPUT_RANK] = {4, 2};
    Tensor* tensor = tensor_create(shape, INPUT_RANK);

    unsigned int indices[INPUT_RANK] = {1, 1};
    tensor_set_element(tensor, indices, 3.14);

    // @note Handle implicit conversion from float to double.
    float element = tensor_get_element(tensor, indices);
    printf("Value at (1, 1): %f\n", (double) element);

    tensor_free(tensor);
    return 0;
}
