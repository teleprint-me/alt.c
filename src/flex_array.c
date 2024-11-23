/**
 * @file src/flex_array.c
 */

#include "flex_array.h"
#include "logger.h"

#include <stdlib.h>

// Create a new flexible array
FlexArray* flex_array_create(unsigned int initial_capacity, DataType type, unsigned int element_size) {
    // Allocate memory for the FlexArray structure
    FlexArray* array = (FlexArray*) malloc(sizeof(FlexArray));
    if (!array) {
        return NULL;
    }

    // Allocate memory for the array's data
    array->data = malloc(initial_capacity * element_size);
    if (!array->data) {
        free(array);
        return NULL;
    }

    // Initialize member variables
    array->element_size = element_size;
    array->length = 0;
    array->capacity = initial_capacity;
    array->type = type;

    // Assign method pointers
    array->append = flex_array_append;
    array->get = flex_array_get;
    array->set = flex_array_set;
    array->pop = flex_array_pop;

    return array;
}

// Free the array
void flex_array_destroy(FlexArray* array) {
    if (array) {
        if (array->data) {
            free(array->data); // Free the memory allocated for data
        }
        free(array); // Free the structure itself
    }
}

int flex_null_guard(FlexArray* array) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Array is null
    }
    return FLEX_ARRAY_SUCCESS;
}

int flex_capacity_guard(FlexArray* array, unsigned int capacity) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Array is null
    }
    if (array->capacity == capacity) {
        return FLEX_ARRAY_SUCCESS; // No resizing needed
    }
    return FLEX_ARRAY_RESIZE;
}

int flex_bounds_guard(FlexArray* array, unsigned int index) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Array is null
    }
    if (index >= array->length) {
        return FLEX_ARRAY_OUT_OF_BOUNDS; // Index out of bounds
    }
    return FLEX_ARRAY_SUCCESS;
}

// Resize the array
FlexState flex_array_resize(FlexArray* array, unsigned int new_capacity) {
    if (!array || new_capacity == 0) {
        return FLEX_ARRAY_ERROR; // Invalid parameters
    }

    if (array->capacity == new_capacity) {
        return FLEX_ARRAY_SUCCESS; // No need to resize
    }

    void* new_data = realloc(array->data, new_capacity * array->element_size);
    if (!new_data) {
        return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED; // Allocation failed
    }

    array->data = new_data;
    array->capacity = new_capacity;
    if (array->length > new_capacity) {
        array->length = new_capacity; // Adjust length if reduced
    }
    return FLEX_ARRAY_SUCCESS;
}

// Get an element at a specific index
FlexState flex_array_get(FlexArray* array, unsigned int index, void* element) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Null pointer
    }
    if (index >= array->length) {
        return FLEX_ARRAY_OUT_OF_BOUNDS; // Out of bounds
    }
    memcpy((char*)array->data + (index * array->element_size), element, array->element_size);
    return FLEX_ARRAY_SUCCESS;
}

// Set an element at a specific index
FlexState flex_array_set(FlexArray* array, unsigned int index, void* element) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Null pointer
    }
    if (index >= array->length) {
        return FLEX_ARRAY_OUT_OF_BOUNDS; // Index out of bounds
    }
    memcpy((char*)array->data + (index * array->element_size), element, array->element_size);
    return FLEX_ARRAY_SUCCESS;
}

// Append an element
FlexState flex_array_append(FlexArray* array, void* element) {
    if (!array) {
        return FLEX_ARRAY_ERROR; // Null pointer
    }

    // Resize if necessary
    if (array->length == array->capacity) {
        FlexState resize_state = flex_array_resize(array, array->capacity * 2);
        if (resize_state != FLEX_ARRAY_SUCCESS) {
            return resize_state; // Propagate resize error
        }
    }

    // Append the element
    memcpy((char*)array->data + (array->length * array->element_size), element, array->element_size);
    array->length++;
    return FLEX_ARRAY_SUCCESS;
}

FlexState flex_array_pop(FlexArray* array, void* element) {
    // Step 1: Validate the array
    if (!array) {
        return FLEX_ARRAY_ERROR; // Null pointer
    }

    // Step 2: Check if the array is empty
    if (array->length == 0) {
        return FLEX_ARRAY_OUT_OF_BOUNDS; // No elements to pop
    }

    // Step 3: Get the last element
    unsigned int last_index = array->length - 1;
    void* last_element = (char*)array->data + (last_index * array->element_size);

    // Step 4: Optionally copy the value to element
    if (element) {
        memcpy(element, last_element, array->element_size);
    }

    // Step 5: Update the array metadata
    array->length--;

    // Optional: Resize the array if its usage drops significantly
    if (array->length < array->capacity / 4) {
        flex_array_resize(array, array->capacity / 2);
    }

    return FLEX_ARRAY_SUCCESS;
}
