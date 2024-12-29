/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/hash.c
 *
 * @brief The Hash Interface is designed to provide a minimal mapping between integers and strings
 * much like a dictionary might behave in Python. This interface should allow users to map strings
 * to integers and integers to strings.
 *
 * @note We should expect the user to take responsibility of the memory allocation. The table is
 * simply used as a reference for lookups. This means the basic operations, insert, search, and
 * delete. For convenience, we can enable clearing the table as well.
 */

#include "algorithm/hash.h"

#include "interface/logger.h"

// -------------------- Hash Life-cycle --------------------

HashTable* hash_create_table(uint64_t initial_size, HashType key_type) {
    HashTable* table = (HashTable*) malloc(sizeof(HashTable));
    if (!table) {
        LOG_ERROR("%s: Failed to allocate memory for HashTable.\n", __func__);
        return NULL;
    }

    table->count = 0;
    table->size = initial_size > 0 ? initial_size : 10;
    table->type = key_type;
    switch (table->type) {
        case HASH_TYPE_STRING:
            table->hash = hash_string;
            table->compare = hash_string_compare;
            break;
        case HASH_TYPE_INTEGER:
            table->hash = hash_integer;
            table->compare = hash_integer_compare;
            break;
        default:
            LOG_ERROR("%s: Invalid HashType given.\n", __func__);
            free(table);
            return NULL;
    }

    table->entries = (HashEntry*) calloc(initial_size, sizeof(HashEntry));
    if (!table->entries) {
        LOG_ERROR("%s: Failed to allocate memory for HashTable entries.\n", __func__);
        free(table);
        return NULL;
    }

    return table;
}

void hash_free_table(HashTable* table) {
    if (table) {
        if (table->entries) {
            free(table->entries);
        }
        free(table);
        table = NULL;
    }
}

// -------------------- Hash Functions --------------------

HashState hash_insert(HashTable* table, const void* key, void* value) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (!key) {
        LOG_ERROR("%s: Key is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (!value) {
        LOG_ERROR("%s: Value is NULL.\n", __func__);
        return HASH_ERROR;
    }

    // Resize if the load factor exceeds 0.75
    if ((double) table->count / table->size > 0.75) {
        if (hash_resize(table, table->size * 2) != HASH_SUCCESS) {
            return HASH_ERROR;
        }
    }

    for (uint64_t i = 0; i < table->size; i++) {
        uint64_t index = table->hash(key, table->size, i);

        if (!table->entries[index].key) { // Empty slot
            table->entries[index].key = (void*) key;
            table->entries[index].value = value;
            table->count++;
            return HASH_SUCCESS;
        } else if (table->compare(table->entries[index].key, key) == 0) { // Duplicate key
            LOG_ERROR("%s: Duplicate key detected.\n", __func__);
            return HASH_KEY_EXISTS;
        }
    }

    LOG_ERROR("%s: Hash table overflow.\n", __func__);
    return HASH_TABLE_FULL;
}

HashState hash_resize(HashTable* table, uint64_t new_size) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (!table->entries) {
        LOG_ERROR("%s: Tables entries is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (0 == table->size) {
        LOG_ERROR("%s: Table size is 0.\n", __func__);
        return HASH_ERROR;
    }
    if (new_size <= table->size) {
        LOG_ERROR("%s: New table size is too small.\n", __func__);
        return HASH_ERROR;
    }

    // Allocate new table entries
    HashEntry* new_entries = (HashEntry*) calloc(new_size, sizeof(HashEntry));
    if (!new_entries) {
        LOG_ERROR("%s: Failed to allocate memory for resized table.\n", __func__);
        return HASH_ERROR;
    }

    // Save the old table
    HashEntry* old_entries = table->entries;
    uint64_t old_size = table->size;

    // Update table properties
    table->entries = new_entries;
    table->size = new_size;

    // Rehash old entries into the new table
    uint64_t rehashed_count = 0;
    for (uint64_t i = 0; i < old_size; i++) {
        HashEntry* entry = &old_entries[i];
        if (entry->key) {
            HashState state = hash_insert(table, entry->key, entry->value);
            if (state != HASH_SUCCESS) {
                LOG_ERROR("%s: Failed to rehash key during resize.\n", __func__);
                free(new_entries);
                table->entries = old_entries;
                table->size = old_size;
                return state;
            }
            rehashed_count++;
        }
    }

    // Update table count
    table->count = rehashed_count;
    // Free old table entries
    free(old_entries);

    return HASH_SUCCESS;
}

HashState hash_delete(HashTable* table, const void* key) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (!key) {
        LOG_ERROR("%s: Key is NULL.\n", __func__);
        return HASH_ERROR;
    }

    for (uint64_t i = 0; i < table->size; i++) {
        uint64_t index = table->hash(key, table->size, i);
        HashEntry* entry = &table->entries[index];

        if (!entry->key) {
            // Key not found, stop probing
            return HASH_KEY_NOT_FOUND;
        }

        if (table->compare(entry->key, key) == 0) {
            // Key found, mark as deleted
            entry->key = NULL;
            entry->value = NULL;
            table->count--;

            // Rehash subsequent entries in the probe sequence
            for (uint64_t j = i + 1; j < table->size; j++) {
                uint64_t rehash_index = table->hash(key, table->size, j);
                HashEntry* rehash_entry = &table->entries[rehash_index];

                if (!rehash_entry->key) {
                    // Stop rehashing when an empty slot is reached
                    break;
                }

                void* rehash_key = rehash_entry->key;
                void* rehash_value = rehash_entry->value;
                rehash_entry->key = NULL;
                rehash_entry->value = NULL;
                table->count--;

                // Reinsert the entry to its correct position in the table
                hash_insert(table, rehash_key, rehash_value);
            }

            return HASH_SUCCESS;
        }
    }

    LOG_DEBUG("%s: Key not found: %s.\n", __func__, (char*) key);
    return HASH_KEY_NOT_FOUND; // Key not found after full probing
}

HashState hash_clear(HashTable* table) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return HASH_ERROR;
    }
    if (!table->entries) {
        LOG_ERROR("%s: Table entries are NULL.\n", __func__);
        return HASH_ERROR;
    }

    for (uint64_t i = 0; i < table->size; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->key) {
            // Clear the entry
            entry->key = NULL;
            entry->value = NULL;
        }
    }

    table->count = 0; // Reset the count
    return HASH_SUCCESS;
}

void* hash_search(HashTable* table, const void* key) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return NULL;
    }
    if (!key) {
        LOG_ERROR("%s: Key is NULL.\n", __func__);
        return NULL;
    }

    for (uint64_t i = 0; i < table->size; i++) {
        uint64_t index = table->hash(key, table->size, i);

        HashEntry* entry = &table->entries[index];
        if (entry->key == NULL) {
            return NULL; // Not found
        }

        if (table->compare(entry->key, key) == 0) {
            return entry->value; // Found
        }
    }

    LOG_DEBUG("%s: Key not found: %s.\n", __func__, (char*) key);
    return NULL; // Not found
}

// ------------------- Hash Integers -------------------

uint64_t hash_integer(const void* key, uint64_t size, uint64_t i) {
    const int32_t* k = (int32_t*) key;
    uint64_t hash = *k * 2654435761U; // Knuth's multiplicative method
    return (hash + i) % size;
}

int hash_integer_compare(const void* key1, const void* key2) {
    return *(const int32_t*) key1 == *(const int32_t*) key2;
}

int32_t* hash_integer_search(HashTable* table, const void* key) {
    return (int32_t*) hash_search(table, key);
}

// ------------------- Hash Strings -------------------

uint64_t hash_djb2(const uint8_t* string) {
    uint64_t hash = 5381;
    int c;

    while ((c = *string++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

uint64_t hash_string(const void* key, uint64_t size, uint64_t i) {
    const uint8_t* string = (const uint8_t*) key;
    return (hash_djb2(string) + i) % size;
}

int hash_string_compare(const void* key1, const void* key2) {
    return strcmp((const char*) key1, (const char*) key2);
}

char* hash_string_search(HashTable* table, const void* key) {
    return (char*) hash_search(table, key);
}
