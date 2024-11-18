/**
 * @file examples/tensor.c
 */

#include "tensors.h"

#include <stdio.h>

int main() {
    unsigned int shape[2] = {4, 2};
    Tensor* tensor = tensor_create(shape, 2);

    unsigned int indices[2] = {1, 1};
    tensor_set(tensor, indices, 3.14);

    printf("Value at (1, 1): %f\n", tensor_get(tensor, indices));

    tensor_free(tensor);
    return 0;
}
