/**
 * @file examples/array.c
 * @brief This is toy example for handling arrays and related manipulations.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct Array
 * @brief A generic array structure.
 */
typedef struct Array {
    uint32_t length; /**< Number of elements in the array */
    uint32_t element_size; /**< Size of each element in bytes */
    void* elements; /**< Pointer to the array's elements */
} Array;

/**
 * @brief Creates a new array with the specified length and element size.
 *
 * @param length Number of elements in the array.
 * @param element_size Size of each element in bytes.
 * @return Pointer to the created Array or NULL on failure.
 */
Array* create_array(uint32_t length, uint32_t element_size) {
    if (length == 0 || element_size == 0) {
        return NULL;
    }

    Array* array = (Array*) malloc(sizeof(Array));
    if (!array) {
        return NULL;
    }

    array->elements = malloc(length * element_size);
    if (!array->elements) {
        free(array);
        return NULL;
    }

    array->length = length;
    array->element_size = element_size;

    return array;
}

/**
 * @brief Frees an array and its associated memory.
 *
 * @param array Pointer to the array to free.
 */
void free_array(Array* array) {
    if (!array) {
        return;
    }
    free(array->elements);
    free(array);
}

/**
 * @brief Resizes the array to the specified length.
 *
 * Existing data is preserved, and new memory is allocated as needed.
 *
 * @param array Pointer to the array.
 * @param new_length New number of elements.
 * @return 0 on success, -1 on failure.
 */
int resize_array(Array* array, uint32_t new_length) {
    if (!array || new_length == 0) {
        return -1;
    }

    void* new_elements = realloc(array->elements, new_length * array->element_size);
    if (!new_elements) {
        return -1;
    }

    array->elements = new_elements;
    array->length = new_length;
    return 0;
}

/**
 * @brief Retrieves an element from the array at the specified index.
 *
 * @param array Pointer to the array.
 * @param index Index of the element to retrieve.
 * @param element Pointer to store the retrieved element.
 * @return 0 on success, -1 on failure.
 */
int get_element(const Array* array, uint32_t index, void* element) {
    if (!array || !element || index >= array->length) {
        return -1;
    }
    memcpy(element, (char*) array->elements + index * array->element_size, array->element_size);
    return 0;
}

/**
 * @brief Sets an element in the array at the specified index.
 *
 * @param array Pointer to the array.
 * @param index Index of the element to set.
 * @param element Pointer to the value to set.
 * @return 0 on success, -1 on failure.
 */
int set_element(Array* array, uint32_t index, const void* element) {
    if (!array || !element || index >= array->length) {
        return -1;
    }
    memcpy((char*) array->elements + index * array->element_size, element, array->element_size);
    return 0;
}

/**
 * @brief Sets multiple elements in the array from a raw buffer.
 *
 * @param array Pointer to the Array.
 * @param data Pointer to the source buffer containing the elements.
 * @param length Number of elements in the source buffer.
 * @return 0 on success, -1 on failure.
 */
int set_elements_bulk(Array* array, const void* data, uint32_t length) {
    if (!array || !data || length > array->length) {
        return -1;
    }
    memcpy(array->elements, data, length * array->element_size);
    return 0;
}

/**
 * @brief Retrieves multiple elements from the array into a raw buffer.
 *
 * @param array Pointer to the Array.
 * @param data Pointer to the destination buffer.
 * @param length Number of elements to retrieve.
 * @return 0 on success, -1 on failure.
 */
int get_elements_bulk(const Array* array, void* data, uint32_t length) {
    if (!array || !data || length > array->length) {
        return -1;
    }
    memcpy(data, array->elements, length * array->element_size);
    return 0;
}

/**
 * @brief Prints the array elements (assuming int for simplicity).
 *
 * @param array Pointer to the array.
 */
void print_array(const Array* array) {
    if (!array || array->element_size != sizeof(int)) {
        printf("Cannot print array. Ensure it's an array of integers.\n");
        return;
    }

    for (uint32_t i = 0; i < array->length; i++) {
        int value;
        get_element(array, i, &value);
        printf("%d ", value);
    }
    printf("\n");
}

/**
 * @brief Example usage of the Array API.
 */
int main(void) {
    Array* int_array = create_array(5, sizeof(int));
    if (!int_array) {
        printf("Failed to create array.\n");
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        set_element(int_array, i, &i);
    }

    printf("Array elements: ");
    print_array(int_array);

    int new_value = 42;
    set_element(int_array, 2, &new_value);
    printf("After update: ");
    print_array(int_array);

    resize_array(int_array, 10);
    printf("After resize: ");
    print_array(int_array);

    free_array(int_array);
    return 0;
}
