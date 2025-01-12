/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/hash.h
 * @brief Minimalistic hash table implementation providing mapping between integers and strings.
 *
 * The Hash Interface is designed to provide a minimal mapping between integers and strings,
 * much like a dictionary in Python. Users can map strings to integers and integers to strings,
 * supporting insertion, search, deletion, and table clearing.
 *
 * @note Comparison functions used with the HashTable must:
 * - Return 0 for equality.
 * - Return a non-zero value for inequality.
 */

#ifndef ALT_HASH_TABLE_H
#define ALT_HASH_TABLE_H

#include <stdint.h>

// ---------------------- Enumerations ----------------------

/**
 * @enum HashTableState
 * @brief Enumerates possible outcomes of hash operations.
 */
typedef enum HashTableState {
    HASH_SUCCESS, /**< Operation completed successfully. */
    HASH_ERROR, /**< General error during the operation. */
    HASH_KEY_EXISTS, /**< Attempted to insert a duplicate key. */
    HASH_KEY_NOT_FOUND, /**< Key not found in the hash table. */
    HASH_TABLE_FULL /**< Hash table is full and cannot insert. */
} HashTableState;

/**
 * @enum HashTableType
 * @brief Represents the key type for the hash table.
 */
typedef enum HashTableType {
    HASH_TYPE_INTEGER, /**< Keys are integers. */
    HASH_TYPE_STRING /**< Keys are strings. */
} HashTableType;

// ---------------------- Structures ----------------------

/**
 * @struct HashTableEntry
 * @brief Represents a single key-value pair in the hash table.
 */
typedef struct HashTableEntry {
    void* key; /**< Pointer to the key. */
    void* value; /**< Pointer to the value. */
} HashTableEntry;

/**
 * @struct HashTable
 * @brief Represents the hash table.
 */
typedef struct HashTable {
    uint64_t count; /**< Number of entries in the table. */
    uint64_t size; /**< Total capacity of the table. */
    HashTableType type; /**< Key type (integer or string). */
    HashTableEntry* entries; /**< Array of hash entries. */
    uint64_t (*hash)(const void* key, uint64_t size, uint64_t i); /**< Hash function. */
    int (*compare)(const void* key1, const void* key2); /**< Comparison function. */
} HashTable;

// -------------------- Hash Life-cycle --------------------

/**
 * @brief Creates a new hash table.
 *
 * @param initial_size Initial size of the hash table.
 * @param key_type Type of keys the table will store (integer or string).
 * @return Pointer to the newly created hash table, or NULL on failure.
 */
HashTable* hash_create_table(uint64_t initial_size, HashTableType key_type);

/**
 * @brief Frees the memory associated with the hash table.
 *
 * @param table Pointer to the hash table to free.
 */
void hash_free_table(HashTable* table);

// -------------------- Hash Functions --------------------

/**
 * @brief Inserts a key-value pair into the hash table.
 *
 * @param table Pointer to the hash table.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @return HASH_SUCCESS on success, or an appropriate error code.
 */
HashTableState hash_insert(HashTable* table, const void* key, void* value);

/**
 * @brief Resizes the hash table to a new size.
 *
 * @param table Pointer to the hash table.
 * @param new_size The new size of the hash table.
 * @return HASH_SUCCESS on success, or HASH_ERROR on failure.
 */
HashTableState hash_resize(HashTable* table, uint64_t new_size);

/**
 * @brief Deletes a key-value pair from the hash table.
 *
 * @param table Pointer to the hash table.
 * @param key Pointer to the key to delete.
 * @return HASH_SUCCESS on success, or HASH_KEY_NOT_FOUND if the key is not found.
 */
HashTableState hash_delete(HashTable* table, const void* key);

/**
 * @brief Clears all entries in the hash table.
 *
 * @param table Pointer to the hash table.
 * @return HASH_SUCCESS on success, or HASH_ERROR on failure.
 */
HashTableState hash_clear(HashTable* table);

/**
 * @brief Searches for a key in the hash table.
 *
 * @param table Pointer to the hash table.
 * @param key Pointer to the key to search.
 * @return Pointer to the associated value, or NULL if the key is not found.
 */
void* hash_search(HashTable* table, const void* key);

// ------------------- Hash Integers -------------------

/**
 * @brief Hash function for integer keys.
 *
 * @param key Pointer to the integer key.
 * @param size Size of the hash table.
 * @param i Probe number for collision resolution.
 * @return Hash value for the given key.
 */
uint64_t hash_integer(const void* key, uint64_t size, uint64_t i);

/**
 * @brief Compares two integer keys for equality.
 *
 * @param key1 Pointer to the first integer key.
 * @param key2 Pointer to the second integer key.
 * @return 0 if the keys are equal, non-zero otherwise.
 */
int hash_integer_compare(const void* key1, const void* key2);

/**
 * @brief Searches for an integer key in the hash table.
 *
 * @param table Pointer to the hash table.
 * @param key Pointer to the integer key to search.
 * @return Pointer to the associated value, or NULL if the key is not found.
 */
int32_t* hash_integer_search(HashTable* table, const void* key);

// ------------------- Hash Strings -------------------

/**
 * @brief Hash function for string keys (DJB2 algorithm).
 *
 * @param string Pointer to the null-terminated string key.
 * @return Hash value for the given string.
 */
uint64_t hash_djb2(const char* string);

/**
 * @brief Hash function for string keys.
 *
 * @param key Pointer to the string key.
 * @param size Size of the hash table.
 * @param i Probe number for collision resolution.
 * @return Hash value for the given key.
 */
uint64_t hash_string(const void* key, uint64_t size, uint64_t i);

/**
 * @brief Compares two string keys for equality.
 *
 * @param key1 Pointer to the first string key.
 * @param key2 Pointer to the second string key.
 * @return 0 if the keys are equal, non-zero otherwise.
 */
int hash_string_compare(const void* key1, const void* key2);

/**
 * @brief Searches for a string key in the hash table.
 *
 * @param table Pointer to the hash table.
 * @param key Pointer to the string key to search.
 * @return Pointer to the associated value, or NULL if the key is not found.
 */
char* hash_string_search(HashTable* table, const void* key);

#endif // ALT_HASH_TABLE_H
