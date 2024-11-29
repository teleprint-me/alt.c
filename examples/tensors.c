/**
 * @file examples/tensor.c
 *
 * @brief Demonstrates basic usage of the tensor library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "data_types.h"
#include "flex_array.h"
#include "tensors.h"

#define INPUT_RANK 2
#define INPUT_SHAPE {4, 2}

int main() {
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Define shape and create FlexArray for it
    uint32_t shape_data[INPUT_RANK] = INPUT_SHAPE;
    FlexArray* shape = flex_array_create(INPUT_RANK, TYPE_UINT32);
    if (!shape || flex_array_set_bulk(shape, shape_data, INPUT_RANK) != FLEX_ARRAY_SUCCESS) {
        fprintf(stderr, "Failed to create or initialize shape FlexArray.\n");
        return EXIT_FAILURE;
    }

    // Create a 2D tensor for floats (tensor takes ownership of shape)
    Tensor* tensor = tensor_create(shape, INPUT_RANK, TYPE_FLOAT32);
    if (!tensor) {
        fprintf(stderr, "Failed to create tensor.\n");
        return EXIT_FAILURE;
    }

    // Set a specific element in the tensor
    uint32_t indices[INPUT_RANK] = {1, 1};
    float value = 3.14f;
    if (tensor_set_element(tensor, (FlexArray*)&(FlexArray){.data = indices, .length = INPUT_RANK}, &value) != TENSOR_SUCCESS) {
        fprintf(stderr, "Failed to set tensor element.\n");
        tensor_free(tensor);
        return EXIT_FAILURE;
    }

    // Get the value back from the tensor
    float retrieved_value = 0.0f;
    if (tensor_get_element(tensor, (FlexArray*)&(FlexArray){.data = indices, .length = INPUT_RANK}, &retrieved_value) != TENSOR_SUCCESS) {
        fprintf(stderr, "Failed to get tensor element.\n");
        tensor_free(tensor);
        return EXIT_FAILURE;
    }

    printf("Value at (1, 1): %f\n", (double) retrieved_value);

    // Fill tensor with random values for demonstration
    printf("Populating tensor with random values...\n");
    for (uint32_t i = 0; i < shape_data[0]; ++i) {
        for (uint32_t j = 0; j < shape_data[1]; ++j) {
            indices[0] = i;
            indices[1] = j;
            float random_value = (float)rand() / (float)(RAND_MAX / 10.0f);
            tensor_set_element(tensor, (FlexArray*)&(FlexArray){.data = indices, .length = INPUT_RANK}, &random_value);
        }
    }

    // Print the entire tensor
    printf("Tensor values:\n");
    for (uint32_t i = 0; i < shape_data[0]; ++i) {
        for (uint32_t j = 0; j < shape_data[1]; ++j) {
            indices[0] = i;
            indices[1] = j;
            tensor_get_element(tensor, (FlexArray*)&(FlexArray){.data = indices, .length = INPUT_RANK}, &retrieved_value);
            printf("%6.2f ", (double) retrieved_value);
        }
        printf("\n");
    }

    // Clean up
    tensor_free(tensor);
    printf("Tensor memory successfully freed.\n");

    return EXIT_SUCCESS;
}
