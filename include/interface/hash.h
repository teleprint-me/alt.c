/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/hash.h
 *
 * @brief The Hash Interface is designed to provide a minimal mapping between integers and strings
 * much like a dictionary might behave in Python. This interface should allow users to map strings
 * to integers and integers to strings.
 * 
 * We assume that a key or value can be either a byte or numerical representation where a byte maybe
 * be literal or symbolic. In the symbolic case, a byte would be char*. This means that int8_t and
 * uint8_t are equivalent to char and unsigned char. This allows us to leverage the builtin types
 * without be burdened by the verbosity of the underlying types. We should expect the user to take
 * responsibility of the memory allocation. The table is simply used as a reference for lookups.
 * This means the basic operations, insert, search, and delete. For convenience, we can enable
 * clearing the table as well.
 *
 * @note Should probably use signed integers since tables can unintentionally overflow.
 * e.g. int64_t instead of uint64_t which is equivalent to size_t. They're all un/signed long. The
 * difference here is that unsigned values can overflow while signed values will wrap.
 */

#ifndef ALT_HASH_H
#define ALT_HASH_H

#include <stdint.h>

// ---------------------- Enumerations ----------------------

typedef enum HashState {
    HASH_SUCCESS,
    HASH_ERROR,
    HASH_KEY_EXISTS,
    HASH_KEY_NOT_FOUND,
    HASH_TABLE_FULL
} HashState;

typedef enum {
    HASH_TYPE_INTEGER,
    HASH_TYPE_STRING
} HashType;

// ---------------------- Structures ----------------------

typedef struct HashEntry {
    void* key; // Pointer to the key
    void* value; // Pointer to the value
} HashEntry;

typedef struct HashTable {
    uint64_t count;
    uint64_t size;
    HashType type;
    HashEntry* entries;
    uint64_t (*hash)(const void* key, uint64_t size, uint64_t i);
    int (*compare)(const void* key1, const void* key2);
} HashTable;

// -------------------- Hash Life-cycle --------------------

HashTable* hash_create_table(uint64_t initial_size, HashType key_type);
void hash_free_table(HashTable* table);

// -------------------- Hash Functions --------------------

HashState hash_insert(HashTable* table, const void* key, void* value);
HashState hash_resize(HashTable* table, uint64_t new_size);
HashState hash_delete(HashTable* table, const void* key);
HashState hash_clear(HashTable* table);
void* hash_search(HashTable* table, const void* key);

// ------------------- Hash Integers -------------------

uint64_t hash_integer(const void* key, uint64_t size, uint64_t i);
int hash_integer_compare(const void* key1, const void* key2);
int32_t* hash_integer_search(HashTable* table, const void* key);

// ------------------- Hash Strings -------------------

uint64_t hash_djb2(const uint8_t* string);
uint64_t hash_string(const void* key, uint64_t size, uint64_t i);
int hash_string_compare(const void* key1, const void* key2);
int8_t* hash_string_search(HashTable* table, const void* key);

#endif // ALT_HASH_H
