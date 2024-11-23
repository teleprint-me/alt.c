/**
 * @file examples/flex_array.c
 */

#include "flex_array.h"

#include <stdio.h>

int main() {
    // Step 1: Create a FlexArray for floats
    FlexArray* array = flex_array_create(5, TYPE_FLOAT32, sizeof(float));
    if (!array) {
        fprintf(stderr, "Failed to create FlexArray.\n");
        return -1;
    }

    // Step 2: Append some float values
    float values[] = {1.1f, 2.2f, 3.3f};
    for (int i = 0; i < 3; i++) {
        if (array->append(array, &values[i]) != FLEX_ARRAY_SUCCESS) {
            fprintf(stderr, "Failed to append value %f.\n", (double) values[i]);
        }
        // Get the element from the array
        float element = 0.0f;
        array->get(array, i, &element);
        printf("Element %u: %f\n", i, (double) element);
    }

    // Step 3: Pop values from the array
    float popped_value = 0.0f;
    while (array->length > 0) {
        if (array->pop(array, &popped_value) == FLEX_ARRAY_SUCCESS) {
            printf("Popped: %f\n", (double) popped_value);
        } else {
            fprintf(stderr, "Failed to pop from FlexArray.\n");
        }
    }

    // Step 4: Destroy the array
    flex_array_destroy(array);
    printf("FlexArray successfully destroyed.\n");

    return 0;
}
