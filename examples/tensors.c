/**
 * @file examples/tensor.c
 *
 * @brief Demonstrates basic usage of the tensor library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "logger.h"
#include "data_types.h"
#include "flex_array.h"
#include "tensors.h"

#define INPUT_RANK 2
#define INPUT_ROWS 4
#define INPUT_COLS 2

// Helper function to print a tensor
void print_tensor(Tensor* tensor) {
    FlexArray* indices = tensor_create_indices(tensor->rank, 0, 0);
    float value = 0.0f;

    if (!indices) {
        LOG_ERROR("Failed to allocate memory for indices.\n");
        return;
    }

    printf("Tensor values:\n");
    for (uint32_t i = 0; i < ((uint32_t*)tensor->shape->data)[0]; ++i) {
        for (uint32_t j = 0; j < ((uint32_t*)tensor->shape->data)[1]; ++j) {
            ((uint32_t*)indices->data)[0] = i;
            ((uint32_t*)indices->data)[1] = j;
            if (tensor_get_element(tensor, indices, &value) == TENSOR_SUCCESS) {
                LOG_INFO("%6.2f ", (double)value);
            } else {
                LOG_ERROR(" ERR  "); // Print error placeholder
            }
        }
        printf("\n");
    }

    flex_array_free(indices);
}

int main() {
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Create a 2D tensor for floats
    Tensor* tensor = tensor_create(TYPE_FLOAT32, INPUT_RANK, INPUT_ROWS, INPUT_COLS);
    if (!tensor) {
        fprintf(stderr, "Failed to create tensor.\n");
        return EXIT_FAILURE;
    }

    // Set a specific element in the tensor
    FlexArray* indices = tensor_create_indices(INPUT_RANK, 1, 1);
    if (!indices) {
        fprintf(stderr, "Failed to create indices.\n");
        tensor_free(tensor);
        return EXIT_FAILURE;
    }

    float value = 3.14f; // Arbitrary value for demonstration
    if (tensor_set_element(tensor, indices, &value) != TENSOR_SUCCESS) {
        fprintf(stderr, "Failed to set tensor element.\n");
        tensor_free(tensor);
        flex_array_free(indices);
        return EXIT_FAILURE;
    }

    // Get the value back from the tensor
    if (tensor_get_element(tensor, indices, &value) == TENSOR_SUCCESS) {
        printf("Value at (1, 1): %6.2f\n", (double)value);
    } else {
        fprintf(stderr, "Failed to get tensor element.\n");
    }

    // Populate tensor with random values
    printf("Populating tensor with random values...\n");
    for (uint32_t i = 0; i < INPUT_ROWS; ++i) {
        for (uint32_t j = 0; j < INPUT_COLS; ++j) {
            ((uint32_t*)indices->data)[0] = i;
            ((uint32_t*)indices->data)[1] = j;
            value = (float)rand() / (float)(RAND_MAX / 10.0f);
            tensor_set_element(tensor, indices, &value);
        }
    }

    // Print the tensor
    print_tensor(tensor);

    // Clean up
    tensor_free(tensor);
    flex_array_free(indices);
    printf("Tensor memory successfully freed.\n");

    return EXIT_SUCCESS;
}
