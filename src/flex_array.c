/**
 * @file src/flex_array.c
 */

#include "flex_array.h"
#include "logger.h"

#include <stdlib.h>

// Create a new flexible array
FlexArray* flex_array_create(unsigned int initial_capacity, DataType type, unsigned int element_size) {
    FlexArray* array = (FlexArray*) malloc(sizeof(FlexArray));
    if (!array) {
        return NULL;
    }

    array->data = malloc(initial_capacity * element_size);
    if (!array->data) {
        free(array);
        return NULL;
    }

    array->element_size = element_size;
    array->length = 0;
    array->capacity = initial_capacity;
    array->type = type;

    array->append = flex_array_append;
    array->get = flex_array_get;
    array->set = flex_array_set;
    array->pop = flex_array_pop;

    return array;
}

// Free the array
void flex_array_destroy(FlexArray* array) {
    if (array) {
        free(array->data);
        free(array);
    }
}

// Resize the array
int flex_array_resize(FlexArray* array, unsigned int new_capacity) {
    void* new_data = realloc(array->data, new_capacity * array->element_size);
    if (!new_data) {
        return FLEX_ARRAY_ERROR; // Allocation failed
    }

    array->data = new_data;
    array->capacity = new_capacity;
    if (array->length > new_capacity) {
        array->length = new_capacity; // Adjust length if reduced
    }
    return FLEX_ARRAY_SUCCESS;
}

// Get an element at a specific index
void* flex_array_get(FlexArray* array, unsigned int index) {
    if (index >= array->length) {
        return NULL; // Index out of bounds
    }
    return (char*) array->data + (index * array->element_size);
}

// Set an element at a specific index
int flex_array_set(FlexArray* array, unsigned int index, void* element) {
    if (index >= array->length) {
        return FLEX_ARRAY_ERROR; // Index out of bounds
    }
    memcpy((char*) array->data + (index * array->element_size), element, array->element_size);
    return FLEX_ARRAY_SUCCESS;
}

// Append an element
int flex_array_append(FlexArray* array, void* element) {
    if (array->length == array->capacity) {
        if (flex_array_resize(array, array->capacity * 2) != FLEX_ARRAY_SUCCESS) {
            return FLEX_ARRAY_ERROR; // Resize failed
        }
    }
    memcpy(
        (char*) array->data + (array->length * array->element_size), element, array->element_size
    );
    array->length++;
    return FLEX_ARRAY_SUCCESS;
}

int flex_array_pop(FlexArray* array, void* element) {
    // Step 1: Check if array is empty
    if (array->length == 0) {
        return FLEX_ARRAY_ERROR;  // Return error code for empty array
    }

    // Step 2: Access the last element
    unsigned int last_index = array->length - 1;
    void* last_element = (char*)array->data + (last_index * array->element_size);

    // Step 3: Copy the value to the output parameter (if not NULL)
    if (element) {
        memcpy(element, last_element, array->element_size);
    }

    // Step 4: Update the array metadata
    array->length--;

    // Optional Step 5: Resize if length is significantly smaller than capacity
    if (array->length < array->capacity / 4) {
        flex_array_resize(array, array->capacity / 2);  // Shrink array
    }

    return FLEX_ARRAY_SUCCESS;  // Return success
}
