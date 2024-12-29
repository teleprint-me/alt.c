/**
 * @file src/interface/hash.c
 */

#include "interface/logger.h"
#include "interface/hash.h"

HashTable* hash_create_table(
    uint64_t initial_size,
    uint64_t (*hash)(const void* key, uint64_t size, uint64_t i),
    int (*compare)(const void* key1, const void* key2)
) {
    HashTable* table = (HashTable*) malloc(sizeof(HashTable));
    if (!table) {
        LOG_ERROR("%s: Failed to allocate memory for HashTable.\n", __func__);
        return NULL;
    }
    table->entries = NULL;
    table->size = initial_size;
    table->count = 0;
    table->hash = hash;
    table->compare = compare;
    return table;
}

void hash_free_table(HashTable* table) {
    if (table) {
        free(table);
    }
}

HashEntry* hash_create_entry(HashTable* table, void* key, void* value) {
    if (!table) {
        LOG_ERROR("%s: Table is NULL.\n", __func__);
        return NULL;
    }
    if (!key) {
        LOG_ERROR("%s: Key is NULL.\n", __func__);
        return NULL;
    }
    if (!value) {
        LOG_ERROR("%s: Value is NULL.\n", __func__);
        return NULL;
    }
    HashEntry* entry = (HashEntry*) malloc(sizeof(HashEntry));
    if (!entry) {
        LOG_ERROR("%s: Failed to allocate memory for HashEntry.\n", __func__);
        return NULL;
    }
    // maybe resize the table and the entry directly to last available element
    table->entries[key] = value; // not sure how handle this yet?
    // update size and count, but resize should probably handle this?
    return entry;
}

void hash_free(HashTable* table) {
    for (size_t i = 0; i < table->size; i++) {
        if (table->table[i].key != NULL) {
            free(table->table[i].key);
            free(table->table[i].value);
        }
    }
    free(table->table);
    free(table);
}

bool hash_insert(HashTable* table, const char* key, Token* value) {
    size_t i = 0;
    while (i < table->size) {
        size_t j = hash(key, table->size, i);
        if (table->table[j].key == NULL) { // Empty slot
            table->table[j].key = strdup(key);
            table->table[j].value = value;
            table->count++;
            return true;
        } else if (strcmp(table->table[j].key, key) == 0) {
            LOG_ERROR("Duplicate key detected: %s", key);
            return false;
        }
        i++;
    }
    LOG_ERROR("Hash table overflow");
    return false;
}

Token* hash_search(HashTable* table, const char* key) {
    size_t i = 0;
    while (i < table->size) {
        size_t j = hash(key, table->size, i);
        if (table->table[j].key == NULL) {
            return NULL; // Not found
        } else if (strcmp(table->table[j].key, key) == 0) {
            return table->table[j].value;
        }
        i++;
    }
    return NULL; // Not found
}

uint64_t djb2(uint8_t* string) {
    int32_t byte;
    uint64_t hash = 5381;

    while (byte = *string++) {
        hash = ((hash << 5) + hash) + byte; // hash * 33 + c
    }

    return hash;
}

uint64_t hash(const char* key, uint64_t size, uint64_t i) {
    uint64_t hash1 = djb2(key); // Primary hash function
    uint64_t hash2 = 1 + (hash1 % (size - 1)); // Secondary hash for double hashing
    return (hash1 + i * hash2) % size;
}
