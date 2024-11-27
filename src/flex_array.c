/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/flex_array.c
 */

#include "flex_array.h"
#include "logger.h"

#include <stdlib.h>

// Create a new flexible array
FlexArray* flex_array_create(uint32_t initial_capacity, DataTypeId id) {
    // Validate the initial capacity
    if (initial_capacity == 0) {
        initial_capacity = 1; // Default minimum capacity
        LOG_WARN("Initial capacity set to default value: 1.");
    }

    // Validate the DataTypeId
    const DataType* type = data_type_get(id);
    if (!type) {
        LOG_ERROR("Invalid DataTypeId passed to %s: id=%u.", __func__, id);
        return NULL;
    }

    // Allocate memory for the FlexArray structure
    FlexArray* array = (FlexArray*) malloc(sizeof(FlexArray));
    if (!array) {
        LOG_ERROR("Failed to allocate memory for FlexArray structure.");
        return NULL;
    }

    // Allocate memory for the array's data
    array->data = malloc(initial_capacity * type->size);
    if (!array->data) {
        LOG_ERROR("Failed to allocate memory for FlexArray data.");
        free(array);
        return NULL;
    }

    // Initialize the FlexArray structure
    array->length = 0;
    array->capacity = initial_capacity;
    array->type = type;
    memset(array->data, 0, initial_capacity * type->size);

    LOG_DEBUG("FlexArray created with initial capacity: %u, type: %s.", initial_capacity, type->name);
    return array;
}

// Free the array
void flex_array_free(FlexArray* array) {
    if (array) {
        if (array->data) {
            free(array->data); // Free the memory allocated for data
        }
        free(array); // Free the structure itself
    }
}

// Resize the array
FlexState flex_array_resize(FlexArray* array, unsigned int new_capacity) {
    if (!array || new_capacity == 0) {
        LOG_ERROR("Invalid parameters passed to flex_array_resize: array=%p, new_capacity=%u.", array, new_capacity);
        return FLEX_ARRAY_ERROR;
    }

    void* new_data = realloc(array->data, new_capacity * array->type->size);
    if (!new_data) {
        LOG_ERROR("Memory reallocation failed: requested capacity=%u.", new_capacity);
        return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED;
    }

    if (new_capacity > array->capacity) {
        memset((char*)new_data + (array->capacity * array->type->size), 0,
               (new_capacity - array->capacity) * array->type->size);
    }

    array->data = new_data;
    array->capacity = new_capacity;
    if (array->length > new_capacity) {
        LOG_WARN("Array resized to smaller capacity. Truncating length from %u to %u.", array->length, new_capacity);
        array->length = new_capacity;
    }

    LOG_DEBUG("FlexArray resized: new capacity=%u.", new_capacity);
    return FLEX_ARRAY_SUCCESS;
}

// Clear the array
FlexState flex_array_clear(FlexArray* array) {
    if (!array) {
        LOG_ERROR("Attempted to access FlexArray with a null pointer.");
        return FLEX_ARRAY_ERROR;
    }
    array->length = 0;
    memset(array->data, 0, array->capacity * array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Get an element at a specific index
FlexState flex_array_get(FlexArray* array, uint32_t index, void* element) {
    if (!array) {
        LOG_ERROR("Attempted to access FlexArray with a null pointer.");
        return FLEX_ARRAY_ERROR;
    }

    if (index >= array->length) {
        LOG_WARN("Out-of-bounds access attempted: index=%u, length=%u.", index, array->length);
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    memcpy(element, (char*) array->data + (index * array->type->size), array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Set an element at a specific index
FlexState flex_array_set(FlexArray* array, uint32_t index, void* element) {
    if (!array) {
        LOG_ERROR("Attempted to access FlexArray with a null pointer.");
        return FLEX_ARRAY_ERROR;
    }

    if (index >= array->length) {
        LOG_WARN("Out-of-bounds access attempted: index=%u, length=%u.", index, array->length);
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    memcpy((char*) array->data + (index * array->type->size), element, array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Append an element
FlexState flex_array_append(FlexArray* array, void* element) {
    if (!array) {
        LOG_ERROR("Attempted to access FlexArray with a null pointer.");
        return FLEX_ARRAY_ERROR;
    }

    if (array->length == array->capacity) {
        FlexState resize_state = flex_array_resize(array, array->capacity * 2);
        if (resize_state != FLEX_ARRAY_SUCCESS) {
            return resize_state;
        }
    }

    memcpy((char*) array->data + (array->length * array->type->size), element, array->type->size);
    array->length++;
    return FLEX_ARRAY_SUCCESS;
}

// Pop the last element
FlexState flex_array_pop(FlexArray* array, void* element) {
    if (!array || array->length == 0) {
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    uint32_t last_index = array->length - 1;
    void* last_element = (char*) array->data + (last_index * array->type->size);

    if (element) {
        memcpy(element, last_element, array->type->size);
    }
    array->length--;

    if (array->length < array->capacity / 4) {
        flex_array_resize(array, array->capacity / 2);
    }

    return FLEX_ARRAY_SUCCESS;
}

// Bulk set (initialize)
FlexState flex_array_set_bulk(FlexArray* array, const void* data, uint32_t size) {
    if (!array || !data || size == 0) {
        return FLEX_ARRAY_ERROR;
    }

    if (flex_array_resize(array, size) != FLEX_ARRAY_SUCCESS) {
        return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED;
    }

    memcpy(array->data, data, size * array->type->size);
    array->length = size;
    return FLEX_ARRAY_SUCCESS;
}
