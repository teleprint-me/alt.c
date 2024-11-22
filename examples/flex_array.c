/**
 * @file examples/flex_array.c
 */

#include "flex_array.h"

#include <stdio.h>

int main() {
    // Step 1: Create a flexible array for floats
    unsigned int initial_capacity = 10;
    FlexArray* float_array = flex_array_create(initial_capacity, TYPE_FLOAT32, sizeof(float));

    if (!float_array) {
        fprintf(stderr, "Failed to create FlexArray\n");
        return -1;
    }

    // Step 2: Append some float values to the array
    float value1 = 3.14f;
    float value2 = 2.71f;
    float value3 = 1.61f;

    if (flex_array_append(float_array, &value1) != 0 || flex_array_append(float_array, &value2) != 0
        || flex_array_append(float_array, &value3) != 0) {
        fprintf(stderr, "Failed to append to FlexArray\n");
        flex_array_destroy(float_array);
        return -1;
    }

    printf("FlexArray successfully created and populated.\n");
    printf("Current length: %u\n", float_array->length);
    printf("Current capacity: %u\n", float_array->capacity);

    // Step 3: Print the elements
    for (unsigned int i = 0; i < float_array->length; i++) {
        float* element = (float*) flex_array_get(float_array, i);
        if (element) {
            printf("Element %u: %f\n", i, (double) *element);
        } else {
            fprintf(stderr, "Failed to get element at index %u\n", i);
        }
    }
    
    // Pop values one by one
    float popped_value = 0.0f;
    while (float_array->length > 0) {
        if (flex_array_pop(float_array, &popped_value) == FLEX_ARRAY_SUCCESS) {
            printf("Popped: %f\n", (double) popped_value);
        } else {
            fprintf(stderr, "Failed to pop from FlexArray.\n");
        }
    }

    // Step 4: Destroy the array
    flex_array_destroy(float_array);
    printf("FlexArray successfully destroyed.\n");

    return 0;
}
