/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/flex_array.h
 *
 * @brief Dynamic, type-safe array implementation with bulk operations.
 *
 * - Designed for simplicity and performance.
 * - Supports both individual and bulk operations using metadata from DataType.
 */

#ifndef ALT_FLEX_ARRAY_H
#define ALT_FLEX_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data_types.h"

typedef enum FlexState {
    FLEX_ARRAY_SUCCESS, /**< Operation was successful */
    FLEX_ARRAY_ERROR, /**< Generic error */
    FLEX_ARRAY_RESIZE, /**< Resize operation performed */
    FLEX_ARRAY_OUT_OF_BOUNDS, /**< Index out of bounds */
    FLEX_ARRAY_MEMORY_ALLOCATION_FAILED /**< Memory allocation failure */
} FlexState;

typedef struct FlexArray {
    // Member variables
    void* data; /**< Pointer to the array's data */
    uint32_t length; /**< Current number of elements */
    uint32_t capacity; /**< Total allocated capacity */
    const DataType* type; /**< Data type of the elements */
} FlexArray;

// Constructor and Destructor
FlexArray* flex_array_create(uint32_t initial_capacity, DataTypeId id);
void flex_array_free(FlexArray* array);

// Modifiers
FlexState flex_array_resize(FlexArray* array, uint32_t new_capacity);
FlexState flex_array_clear(FlexArray* array);

// Accessors
FlexState flex_array_get(FlexArray* array, uint32_t index, void* element);
FlexState flex_array_set(FlexArray* array, uint32_t index, void* element);
FlexState flex_array_append(FlexArray* array, void* element);
FlexState flex_array_pop(FlexArray* array, void* element);

// Bulk Operations
FlexState flex_array_set_bulk(FlexArray* array, const void* data, uint32_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_FLEX_ARRAY_H
