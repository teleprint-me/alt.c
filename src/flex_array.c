/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/flex_array.c
 *
 * @brief Dynamic, type-safe array implementation with bulk operations.
 *
 * - Provides dynamic resizing and type safety using metadata from DataType.
 * - Supports individual and bulk operations for flexible array management.
 *
 * @note The difficulty level for this is high.
 * @warning Do not underestimate the difficulty in properly managing memory reallocation.
 */

#include "flex_array.h"
#include "logger.h"

#include <stdlib.h>

// Create a new flexible array
FlexArray* flex_array_create(uint32_t capacity, DataTypeId id) {
    // Validate the initial capacity
    if (capacity == 0) {
        capacity = 1; // Default minimum capacity
        LOG_WARN("%s: Initial capacity set to default value of 1.\n", __func__);
    }

    // Validate the DataTypeId
    const DataType* type = data_type_get(id);
    if (!type) {
        LOG_ERROR("%s: Invalid DataTypeId using id=%u.\n", __func__, id);
        return NULL;
    }

    // Allocate memory for the FlexArray structure
    FlexArray* array = (FlexArray*) malloc(sizeof(FlexArray));
    if (!array) {
        LOG_ERROR("%s: Failed to allocate memory for FlexArray structure.\n", __func__);
        return NULL;
    }

    // Allocate memory for the array's data
    array->data = malloc(capacity * type->size);
    if (!array->data) {
        LOG_ERROR("%s: Failed to allocate memory for FlexArray data.\n", __func__);
        free(array);
        return NULL;
    }

    // Initialize the FlexArray structure
    array->length = 0;
    array->capacity = capacity;
    array->type = type;
    memset(array->data, 0, capacity * type->size);

    LOG_DEBUG("%s: Created with initial capacity: %u, type: %s.\n", __func__, capacity, type->name);
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
FlexState flex_array_resize(FlexArray* array, uint32_t capacity) {
    if (!array || capacity == 0) {
        LOG_ERROR("%s: Invalid parameters: array=%p, capacity=%u.\n", __func__, array, capacity);
        return FLEX_ARRAY_ERROR;
    }

    void* new_data = realloc(array->data, capacity * array->type->size);
    if (!new_data) {
        LOG_ERROR("%s: Memory reallocation failed: requested capacity=%u.\n", __func__, capacity);
        return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED;
    }

    if (capacity > array->capacity) {
        memset((char*)new_data + (array->capacity * array->type->size), 0,
               (capacity - array->capacity) * array->type->size);
    }

    array->data = new_data;
    array->capacity = capacity;
    if (array->length > capacity) {
        LOG_WARN("%s: Resized to smaller capacity. Truncating length from %u to %u.\n", __func__, array->length, capacity);
        array->length = capacity;
    }

    LOG_DEBUG("%s: Resized: new capacity=%u.\n", __func__, capacity);
    return FLEX_ARRAY_SUCCESS;
}

// Shrink the array
FlexState flex_array_shrink_to_fit(FlexArray* array) {
    if (!array) {
        return FLEX_ARRAY_ERROR;
    }

    if (array->length < array->capacity) {
        if (flex_array_resize(array, array->length) != FLEX_ARRAY_SUCCESS) {
            return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED;
        }
    }

    return FLEX_ARRAY_SUCCESS;
}

// Clear the array
FlexState flex_array_clear(FlexArray* array) {
    if (!array) {
        LOG_ERROR("%s: Attempted to access FlexArray with a null pointer.\n", __func__);
        return FLEX_ARRAY_ERROR;
    }
    array->length = 0;
    memset(array->data, 0, array->capacity * array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Get an element at a specific index
FlexState flex_array_get(FlexArray* array, uint32_t index, void* element) {
    if (!array) {
        LOG_ERROR("%s: Attempted to access FlexArray with a null pointer.\n", __func__);
        return FLEX_ARRAY_ERROR;
    }

    if (index >= array->length) {
        LOG_WARN("%s: Out-of-bounds access attempted: index=%u, length=%u.\n", __func__, index, array->length);
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    memcpy(element, (char*) array->data + (index * array->type->size), array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Set an element at a specific index
FlexState flex_array_set(FlexArray* array, uint32_t index, void* element) {
    if (!array) {
        LOG_ERROR("%s: Attempted to access FlexArray with a null pointer.\n", __func__);
        return FLEX_ARRAY_ERROR;
    }

    if (index >= array->length) {
        LOG_WARN("%s: Out-of-bounds access attempted: index=%u, length=%u.\n", __func__, index, array->length);
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    memcpy((char*) array->data + (index * array->type->size), element, array->type->size);
    return FLEX_ARRAY_SUCCESS;
}

// Append an element
FlexState flex_array_append(FlexArray* array, void* element) {
    if (!array) {
        LOG_ERROR("%s: Attempted to access FlexArray with a null pointer.\n", __func__);
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
    if (!array || array->length == 0 || !element) {
        LOG_ERROR("%s: Out-of-bounds access attempted: array=%p, length=%u, element=%p.\n", __func__, array, array->length, element);
        return FLEX_ARRAY_OUT_OF_BOUNDS;
    }

    uint32_t last_index = array->length - 1;
    void* last_element = (char*) array->data + (last_index * array->type->size);

    memcpy(element, last_element, array->type->size);
    array->length--;

    if (array->length < array->capacity / 4) {
        flex_array_resize(array, array->capacity / 2);
    }

    return FLEX_ARRAY_SUCCESS;
}

// Bulk set (initialize)
FlexState flex_array_set_bulk(FlexArray* array, const void* data, uint32_t length) {
    if (!array || !data || length == 0) {
        LOG_ERROR("%s: Attempted to access FlexArray with a null pointer.\n", __func__);
        return FLEX_ARRAY_ERROR;
    }

    // Resize only if the new length exceeds the current capacity
    if (length > array->capacity) {
        if (flex_array_resize(array, length) != FLEX_ARRAY_SUCCESS) {
            return FLEX_ARRAY_MEMORY_ALLOCATION_FAILED;
        }
    }

    // If data length is smaller, adjust the array length but do not resize
    memcpy(array->data, data, length * array->type->size);
    array->length = length;

    return FLEX_ARRAY_SUCCESS;
}
