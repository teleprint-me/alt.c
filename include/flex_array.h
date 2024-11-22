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

#define FLEX_ARRAY_SUCCESS 0
#define FLEX_ARRAY_ERROR -1

typedef struct FlexArray {
    // Member variables
    void* data; /**< Pointer to the array's data */
    unsigned int element_size; /**< Size of each element in bytes */
    unsigned int length; /**< Current number of elements */
    unsigned int capacity; /**< Total allocated capacity */
    DataType type; /**< Data type of the elements */

    // Function pointers
    int (*append)(struct FlexArray*, void*);
    int (*pop)(struct FlexArray*, void* element);
    void* (*get)(struct FlexArray*, unsigned int);
    int (*set)(struct FlexArray*, unsigned int, void* element);
} FlexArray;

/* Initialize a new flexible array */
FlexArray* flex_array_create(unsigned int initial_capacity, DataType type, unsigned int element_size);

/* Free the memory of a flexible array */
void flex_array_destroy(FlexArray* array);

/* Resize a flexible array */
int flex_array_resize(FlexArray* array, unsigned int new_capacity);

/* Append an element to the array */
int flex_array_append(FlexArray* array, void* element);

/* Pop an element from the array */
int flex_array_pop(FlexArray* array, void* element);

/* Get an element from the array */
void* flex_array_get(FlexArray* array, unsigned int index);

/* Set an element in the array */
int flex_array_set(FlexArray* array, unsigned int index, void* element);

#endif // ALT_FLEX_ARRAY_H
