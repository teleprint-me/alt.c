/**
 * @file examples/flex_array.c
 */

#include <stdio.h>

#include "interface/logger.h"
#include "interface/flex_array.h"

int main() {
    global_logger.log_level = LOG_LEVEL_DEBUG;

    // Step 1: Create a FlexArray for floats
    FlexArray* array = flex_array_create(5, TYPE_FLOAT32);
    if (!array) {
        fprintf(stderr, "Failed to create FlexArray.\n");
        return -1;
    }
    printf("FlexArray successfully created and populated.\n");
    printf("FlexArray->length = %u;\n", array->length);
    printf("FlexArray->capacity = %u;\n", array->capacity);

    // Step 2: Append some float values
    float element = 0.0f;
    float values[] = {1.1f, 2.2f, 3.3f};
    for (int i = 0; i < 3; i++) {
        if (flex_array_append(array, &values[i]) != FLEX_ARRAY_SUCCESS) {
            fprintf(stderr, "Failed to append value %f.\n", (double) values[i]);
        }
        // Get the element from the array
        flex_array_get(array, i, &element);
        printf("FlexArray->data[%u] = %f;\n", i, (double) element);
    }

    // Step 3: Pop values from the array
    element = 0.0f; // reset
    while (array->length > 0) {
        if (flex_array_pop(array, &element) == FLEX_ARRAY_SUCCESS) {
            printf("Popped: %f\n", (double) element);
        } else {
            fprintf(stderr, "Failed to pop from FlexArray.\n");
        }
    }

    // Step 4: Destroy the array
    flex_array_free(array);
    printf("FlexArray successfully freed.\n");

    return 0;
}
