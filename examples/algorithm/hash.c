/**
 * @file examples/algorithm/hash.c
 * @brief Simple example showcasing Hash API usage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "algorithm/hash.h"

int main(void) {
    // Create a hash table for strings
    HashTable* string_table = hash_create_table(8, HASH_TYPE_STRING);

    if (!string_table) {
        fprintf(stderr, "Failed to create string hash table.\n");
        return 1;
    }

    // Insert string keys and values
    hash_insert(string_table, "hello", "world");
    hash_insert(string_table, "foo", "bar");
    hash_insert(string_table, "baz", "qux");

    // Search for a string key
    char* value = hash_string_search(string_table, "hello");
    if (value) {
        printf("Found key 'hello' with value: %s\n", value);
    } else {
        printf("Key 'hello' not found.\n");
    }

    // Delete a key
    if (hash_delete(string_table, "foo") == HASH_SUCCESS) {
        printf("Deleted key 'foo'.\n");
    } else {
        printf("Failed to delete key 'foo'.\n");
    }

    // Verify deletion
    value = hash_string_search(string_table, "foo");
    if (value) {
        printf("Key 'foo' still exists with value: %s\n", value);
    } else {
        printf("Key 'foo' no longer exists.\n");
    }

    // Clear the table
    if (hash_clear(string_table) == HASH_SUCCESS) {
        printf("Cleared string hash table.\n");
    }

    // Create a hash table for integers
    HashTable* int_table = hash_create_table(8, HASH_TYPE_INTEGER);

    if (!int_table) {
        fprintf(stderr, "Failed to create integer hash table.\n");
        hash_free_table(string_table);
        return 1;
    }

    // Insert integer keys and values
    int key1 = 42, key2 = 99, key3 = 123;
    int value1 = 1, value2 = 2, value3 = 3;
    hash_insert(int_table, &key1, &value1);
    hash_insert(int_table, &key2, &value2);
    hash_insert(int_table, &key3, &value3);

    // Search for an integer key
    int* int_value = hash_integer_search(int_table, &key2);
    if (int_value) {
        printf("Found key %d with value: %d\n", key2, *int_value);
    } else {
        printf("Key %d not found.\n", key2);
    }

    // Delete an integer key
    if (hash_delete(int_table, &key1) == HASH_SUCCESS) {
        printf("Deleted key %d.\n", key1);
    } else {
        printf("Failed to delete key %d.\n", key1);
    }

    // Verify deletion
    int_value = hash_integer_search(int_table, &key1);
    if (int_value) {
        printf("Key %d still exists with value: %d\n", key1, *int_value);
    } else {
        printf("Key %d no longer exists.\n", key1);
    }

    // Free tables
    hash_free_table(string_table);
    hash_free_table(int_table);

    printf("Hash table example complete.\n");
    return 0;
}
