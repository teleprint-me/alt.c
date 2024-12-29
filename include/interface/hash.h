/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/hash.h
 *
 * We assume that a key or value can be either a byte or numerical representation where a byte maybe
 * be literal or symbolic. In the symbolic case, a byte would be char*. This means that int8_t and
 * uint8_t are equivalent to char and unsigned char. This allows us to leverage the builtin types
 * without be burdened by the verbosity of the underlying types. We should expect the user to take
 * responsibility of the memory allocation. The table is simply used as a reference and for lookups.
 * This means the basic operations, insert, search, and delete. For convenience, we can enable
 * clearing the table as well.
 *
 * @note Should probably use signed integers since tables can unintentionally overflow.
 * e.g. int64_t instead of uint64_t which equivalent to size_t. they're all un/signed long. The
 * difference here is that unsigned values can overflow while signed values wrap.
 */

#ifndef ALT_HASH_H
#define ALT_HASH_H

#include <stdint.h>

// ---------------------- Constants and Types ----------------------

typedef enum HashState {
    HASH_SUCCESS,
    HASH_ERROR,
    HASH_KEY_EXISTS,
    HASH_KEY_NOT_FOUND,
    HASH_TABLE_FULL
} HashState;

typedef struct HashEntry {
    void* key; // Pointer to the key
    void* value; // Pointer to the value
} HashEntry;

typedef struct HashTable {
    HashEntry* entries; // Array of hash entries
    uint64_t size; // Current size of the table
    uint64_t count; // Number of elements currently in the table
    uint64_t (*hash)(const void* key, uint64_t size, uint64_t i); // Custom hash function
    int (*compare)(const void* key1, const void* key2); // Custom key comparison function
} HashTable;

// -------------------- Hash Table Functions --------------------

// Hash table life-cycle
HashTable* hash_create_table(
    uint64_t initial_size,
    uint64_t (*hash)(const void* key, uint64_t size, uint64_t i),
    int (*compare)(const void* key1, const void* key2)
);
void hash_free_table(HashTable* table);

// Hash entry life-cycle
HashEntry* hash_create_entry(void* key, void* value);
void hash_free_entry(HashEntry* entry);

// Hash table operations
HashState hash_insert(HashTable* table, const void* key, void* value);
void* hash_search(HashTable* table, const void* key);
HashState hash_delete(HashTable* table, const void* key);
HashState hash_clear(HashTable* table);
HashState hash_resize(HashTable* table, uint64_t new_size);

// ------------------- Utility Hash Functions -------------------

uint64_t hash_djb2(const uint8_t* string);
uint64_t hash_default(const void* key, uint64_t size, uint64_t i);
int hash_compare(const void* key1, const void* key2);

#endif // ALT_HASH_H
