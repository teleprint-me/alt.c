/**
 * @file include/flex_array.h
 *
 * This implementation uses pure C with minimal dependencies on external libraries.
 *
 * - Keep the initial implementation simple.
 * - Only add methods on an as-needed basis.
 */

#ifndef ALT_FLEX_ARRAY_H
#define ALT_FLEX_ARRAY_H

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
    unsigned int element_size; /**< Size of each element in bytes */
    unsigned int length; /**< Current number of elements */
    unsigned int capacity; /**< Total allocated capacity */
    DataType type; /**< Data type of the elements */

    // Function pointers for methods
    FlexState (*append)(struct FlexArray*, void* element); /**< Append an element */
    FlexState (*pop)(struct FlexArray*, void* element); /**< Pop the last element */
    FlexState (*get)(struct FlexArray*, unsigned int index, void* element); /**< Get an element by index */
    FlexState (*set)(struct FlexArray*, unsigned int index, void* element); /**< Set an element */
} FlexArray;

// Constructor and Destructor
FlexArray* flex_array_create(unsigned int initial_capacity, DataType type, unsigned int element_size);
void flex_array_destroy(FlexArray* array);

// Modifiers
FlexState flex_array_append(FlexArray* array, void* element);
FlexState flex_array_resize(FlexArray* array, unsigned int new_capacity);

// Accessors
FlexState flex_array_get(FlexArray* array, unsigned int index, void* element);
FlexState flex_array_set(FlexArray* array, unsigned int index, void* element);
FlexState flex_array_pop(FlexArray* array, void* element);

#endif // ALT_FLEX_ARRAY_H
