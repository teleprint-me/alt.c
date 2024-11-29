/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/flex_array.h
 *
 * @brief Dynamic, type-safe array implementation with bulk operations.
 *
 * - Provides dynamic resizing and type safety using metadata from DataType.
 * - Supports individual and bulk operations for flexible array management.
 */

#ifndef ALT_FLEX_ARRAY_H
#define ALT_FLEX_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "data_types.h"

/**
 * @enum FlexState
 * @brief Return codes for FlexArray operations.
 */
typedef enum FlexState {
    FLEX_ARRAY_SUCCESS, /**< Operation completed successfully */
    FLEX_ARRAY_ERROR, /**< Generic error */
    FLEX_ARRAY_RESIZE, /**< Resize operation completed */
    FLEX_ARRAY_OUT_OF_BOUNDS, /**< Index out of bounds */
    FLEX_ARRAY_MEMORY_ALLOCATION_FAILED /**< Memory allocation failed */
} FlexState;

/**
 * @struct FlexArray
 * @brief A dynamic, type-safe array structure.
 */
typedef struct FlexArray {
    void* data; /**< Pointer to the array's data */
    uint32_t length; /**< Current number of elements */
    uint32_t capacity; /**< Total allocated capacity */
    const DataType* type; /**< Data type of the array elements */
} FlexArray;

/**
 * @brief Creates a new FlexArray with the specified initial capacity and data type.
 *
 * @param initial_capacity Initial number of elements the array can hold.
 * @param id Data type identifier for the array elements.
 * @return Pointer to the created FlexArray or NULL on failure.
 */
FlexArray* flex_array_create(uint32_t initial_capacity, DataTypeId id);

/**
 * @brief Frees a FlexArray and its associated resources.
 *
 * @param array Pointer to the FlexArray to free.
 */
void flex_array_free(FlexArray* array);

/**
 * @brief Resizes the FlexArray to the specified capacity.
 *
 * Existing data is preserved, and new memory is allocated as needed.
 *
 * @param array Pointer to the FlexArray to resize.
 * @param new_capacity New capacity for the array.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_resize(FlexArray* array, uint32_t new_capacity);

/**
 * @brief Clears all elements from the FlexArray without resizing it.
 *
 * @param array Pointer to the FlexArray to clear.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_clear(FlexArray* array);

/**
 * @brief Retrieves an element from the FlexArray at the specified index.
 *
 * @param array Pointer to the FlexArray.
 * @param index Index of the element to retrieve.
 * @param element Pointer to store the retrieved element.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_get(FlexArray* array, uint32_t index, void* element);

/**
 * @brief Sets an element in the FlexArray at the specified index.
 *
 * @param array Pointer to the FlexArray.
 * @param index Index of the element to set.
 * @param element Pointer to the value to set.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_set(FlexArray* array, uint32_t index, void* element);

/**
 * @brief Appends an element to the end of the FlexArray.
 *
 * Resizes the array if necessary to accommodate the new element.
 *
 * @param array Pointer to the FlexArray.
 * @param element Pointer to the value to append.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_append(FlexArray* array, void* element);

/**
 * @brief Removes the last element from the FlexArray and returns it.
 *
 * @param array Pointer to the FlexArray.
 * @param element Pointer to store the removed element.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_pop(FlexArray* array, void* element);

/**
 * @brief Sets multiple elements in the FlexArray from a data buffer.
 *
 * Copies data into the FlexArray. Resizes the array if the data length exceeds its current
 * capacity.
 *
 * @param array Pointer to the FlexArray.
 * @param data Pointer to the buffer containing the data to set.
 * @param length Number of elements to set.
 * @return FlexState indicating the result of the operation.
 */
FlexState flex_array_set_bulk(FlexArray* array, const void* data, uint32_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_FLEX_ARRAY_H
